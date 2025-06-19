class GravitySystem : public System
{
    void OnUpdate(Scene *scene, f32 deltaTime)
    {
        for (EntityID ent: SceneView<Rigidbody, GravityComponent>(*scene))
        {
            Rigidbody *rb = scene->Get<Rigidbody>(ent);
            GravityComponent *gc = scene->Get<GravityComponent>(ent);

            rb->v_y -= gc->strength;
        }
    }
};

// Scans for collision of a single component
// and edits trajectory of ball otherwise
void scanCollision(CircleCollider *checkCollider, Rigidbody *accessRigid, Transform3D *accessTransform, Scene &accessScene)
{
    for (EntityID ent: SceneView<Transform3D, Rigidbody, CircleCollider>(accessScene))
    {
        Transform3D *t = accessScene.Get<Transform3D>(ent);
        Rigidbody *rb = accessScene.Get<Rigidbody>(ent);
        CircleCollider *cc = accessScene.Get<CircleCollider>(ent);
        if (rb != accessRigid)
        {
            double diffX = t->position.y - accessTransform->position.y;
            double diffY = t->position.z - accessTransform->position.z;
            double distance = sqrt(diffX * diffX + diffY * diffY);
            if (distance < cc->radius + checkCollider->radius)
            {
                double normX = diffX / distance;
                double normY = diffY / distance;
                double thisSpeedMag = -sqrt(accessRigid->v_x * accessRigid->v_x + accessRigid->v_y * accessRigid->v_y);
                accessRigid->v_x = normX * thisSpeedMag;
                accessRigid->v_y = normY * thisSpeedMag;
                double speedMag = sqrt(rb->v_x * rb->v_x + rb->v_y * rb->v_y);
                rb->v_x = normX * speedMag;
                rb->v_y = normY * speedMag;
            }
        }
    }
}

class CollisionSystem : public System
{
    void OnUpdate(Scene *scene, f32 deltaTime)
    {
        // Forward movement, collision, rendering
        for (EntityID ent: SceneView<Transform3D, Rigidbody, CircleCollider>(*scene))
        {
            Transform3D *t = scene->Get<Transform3D>(ent);
            Rigidbody *rb = scene->Get<Rigidbody>(ent);
            CircleCollider *cc = scene->Get<CircleCollider>(ent);
            float radius = cc->radius;

            // Not framerate independent for simpler collision logic.
            t->position.y += rb->v_x;
            t->position.z += rb->v_y;

            // Collision check x-axis
            if ((t->position.y - radius) < (WINDOW_WIDTH / -2.0f) || (t->position.y + radius) > (WINDOW_WIDTH / 2.0f))
            {
                rb->v_x *= -1;
            }

            // Collision check y-axis
            if ((t->position.z - radius) < (WINDOW_HEIGHT / -2.0f) || (t->position.z + radius) > (WINDOW_HEIGHT / 2.0f))
            {
                rb->v_y *= -1;
            }

            scanCollision(cc, rb, t, *scene);
        }
    }
};

std::vector<glm::vec4> getFrustumCorners(const glm::mat4& proj, const glm::mat4& view)
{
    glm::mat4 inverse = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (u32 x = 0; x < 2; ++x)
    {
        for (u32 y = 0; y < 2; ++y)
        {
            for (u32 z = 0; z < 2; ++z)
            {
                const glm::vec4 pt =
                        inverse * glm::vec4(
                                    2.0f * x - 1.0f,
                                    2.0f * y - 1.0f,
                                    z,
                                    1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

#define NUM_CASCADES 6

class RenderSystem : public System
{
    CameraID mainCam;

    glm::vec3 ambientLight;

    void OnStart(Scene *scene)
    {
        InitPipelines(NUM_CASCADES);

        mainCam = AddCamera(1);

        ambientLight = {0.1, 0.1, 0.1};
    }

    void OnUpdate(Scene *scene, f32 deltaTime)
    {
        for (EntityID ent: SceneView<DirLight, Transform3D>(*scene))
        {
            DirLight *l = scene->Get<DirLight>(ent);
            if (l->shadowID == -1)
            {
                l->shadowID = CreateDepthArray(2048, 2048, NUM_CASCADES);
            }
            if(l->cameraID == -1)
            {
                l->cameraID = AddCamera(NUM_CASCADES);
            }
        }

        for (EntityID ent: SceneView<SpotLight, Transform3D>(*scene))
        {
            SpotLight *l = scene->Get<SpotLight>(ent);
            if (l->shadowID == -1)
            {
                l->shadowID = CreateDepthTexture(1024, 1024);
            }
            if(l->cameraID == -1)
            {
                l->cameraID = AddCamera(1);
            }
        }

        for (EntityID ent: SceneView<PointLight, Transform3D>(*scene))
        {
            PointLight *l = scene->Get<PointLight>(ent);
            if (l->shadowID == -1)
            {
                l->shadowID = CreateDepthCubemap(512, 512);
            }
            if(l->cameraID == -1)
            {
                l->cameraID = AddCamera(6);
            }
        }

        if (!InitFrame())
        {
            return;
        }

        // 1. Gather counts of each unique mesh pointer.
        std::map<MeshID, u32> meshCounts;
        for (EntityID ent: SceneView<MeshComponent, ColorComponent, Transform3D>(*scene))
        {
            MeshComponent *m = scene->Get<MeshComponent>(ent);
            ++meshCounts[m->mesh];  // TODO: Verify the legitness of this
        }

        // 2. Create, with fixed size, the list of Mat4s, by adding up all of the counts.
        // 3. Get pointers to the start of each segment of unique mesh pointer.
        u32 totalCount = 0;
        std::unordered_map<MeshID, u32> offsets;
        for (std::pair<MeshID, u32> pair: meshCounts)
        {
            offsets[pair.first] = totalCount;
            totalCount += pair.second;
        }

        std::vector<ObjectData> objects(totalCount);

        // 4. Iterate through scene view once more and fill in the fixed size array.
        for (EntityID ent: SceneView<MeshComponent, ColorComponent, Transform3D>(*scene))
        {
            Transform3D *t = scene->Get<Transform3D>(ent);
            glm::mat4 model = GetTransformMatrix(t);
            MeshComponent *m = scene->Get<MeshComponent>(ent);
            MeshID mesh = m->mesh;
            ColorComponent *c = scene->Get<ColorComponent>(ent);

            objects[offsets[mesh]++] = {model, glm::vec4(c->r, c->g, c->b, 1.0f)};
        }

        SendObjectData(objects);

        // Get the main camera view

        SceneView<CameraComponent, Transform3D> cameraView = SceneView<CameraComponent, Transform3D>(*scene);
        if (cameraView.begin() == cameraView.end())
        {
            EndFrame();
            return;
        }

        EntityID cameraEnt = *cameraView.begin();
        CameraComponent *camera = scene->Get<CameraComponent>(cameraEnt);
        Transform3D *cameraTransform = scene->Get<Transform3D>(cameraEnt);
        glm::mat4 view = GetViewMatrix(cameraTransform);
        f32 aspect = (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT;

        glm::mat4 proj = glm::perspective(glm::radians(camera->fov), aspect, camera->near, camera->far);

        // Calculate cascaded shadow views

        CameraData dirViews[NUM_CASCADES];

        f32 subFrustumSize = (camera->far - camera->near) / NUM_CASCADES;

        f32 currentNear = camera->near;

        std::vector<DirLightData> dirLightData;

        std::vector<LightCascade> cascades;

        u32 startIndex;

        for (EntityID dirEnt: SceneView<DirLight, Transform3D>(*scene))
        {
            Transform3D *dirTransform = scene->Get<Transform3D>(dirEnt);
            DirLight *dirLight = scene->Get<DirLight>(dirEnt);
            glm::mat4 dirView = GetViewMatrix(dirTransform);

            for (int i = 0; i < NUM_CASCADES; i++)
            {
                glm::mat4 subProj = glm::perspective(glm::radians(camera->fov), aspect,
                                                     currentNear, currentNear + subFrustumSize);
                currentNear += subFrustumSize;

                f32 minX = std::numeric_limits<f32>::max();
                f32 maxX = std::numeric_limits<f32>::lowest();
                f32 minY = std::numeric_limits<f32>::max();
                f32 maxY = std::numeric_limits<f32>::lowest();
                f32 minZ = std::numeric_limits<f32>::max();
                f32 maxZ = std::numeric_limits<f32>::lowest();

                std::vector<glm::vec4> corners = getFrustumCorners(subProj, view);

                for (const glm::vec3& v : corners)
                {
                    const glm::vec4 trf = dirView * glm::vec4(v, 1.0);
                    minX = std::min(minX, trf.x);
                    maxX = std::max(maxX, trf.x);
                    minY = std::min(minY, trf.y);
                    maxY = std::max(maxY, trf.y);
                    minZ = std::min(minZ, trf.z);
                    maxZ = std::max(maxZ, trf.z);
                }

                glm::mat4 dirProj = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

                dirViews[i] = {dirView, dirProj, {}};

                cascades.push_back({dirProj * dirView, currentNear});
            }

            glm::vec3 lightDir = GetForwardVector(dirTransform);

            BeginCascadedPass(dirLight->shadowID, CullMode::BACK);

            SetCamera(dirLight->cameraID);
            UpdateCamera(NUM_CASCADES, dirViews);

            startIndex = 0;
            for (std::pair<MeshID, u32> pair: meshCounts)
            {
                SetMesh(pair.first);
                DrawObjects(pair.second, startIndex);
                startIndex += pair.second;
            }
            EndPass();

            dirLightData.push_back({GetForwardVector(dirTransform), dirLight->shadowID,
                                    dirLight->diffuse, dirLight->specular});
        }


        std::vector<SpotLightData> spotLightData;

        for (EntityID spotEnt: SceneView<SpotLight, Transform3D>(*scene))
        {
            Transform3D *spotTransform = scene->Get<Transform3D>(spotEnt);
            SpotLight *spotLight = scene->Get<SpotLight>(spotEnt);

            glm::mat4 spotView = GetViewMatrix(spotTransform);
            glm::mat4 spotProj = glm::perspective(glm::radians(spotLight->outerCone * 2), 1.0f, 0.01f, spotLight->range);
            CameraData spotCamData = {spotView, spotProj, spotTransform->position};

            BeginShadowPass(spotLight->shadowID, CullMode::BACK);

            SetCamera(spotLight->cameraID);
            UpdateCamera(1, &spotCamData);

            startIndex = 0;
            for (std::pair<MeshID, u32> pair: meshCounts)
            {
                SetMesh(pair.first);
                DrawObjects(pair.second, startIndex);
                startIndex += pair.second;
            }
            EndPass();

            spotLightData.push_back({spotProj * spotView, spotTransform->position, GetForwardVector(spotTransform),
                                     spotLight->shadowID, spotLight->diffuse, spotLight->specular,
                                     cos(glm::radians(spotLight->innerCone)), cos(glm::radians(spotLight->outerCone)),
                                     spotLight->range});
        }

        std::vector<PointLightData> pointLightData;

        for (EntityID pointEnt: SceneView<PointLight, Transform3D>(*scene))
        {
            Transform3D *pointTransform = scene->Get<Transform3D>(pointEnt);
            PointLight *pointLight = scene->Get<PointLight>(pointEnt);

            glm::vec3 pointPos = pointTransform->position;

            CameraData pointCamData[6];

            glm::mat4 pointProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.5f, pointLight->maxRange);
            glm::mat4 pointViews[6];

            GetPointViews(pointTransform, pointViews);

            for (int i = 0; i < 6; i++)
            {
                pointCamData[i] = {pointViews[i], pointProj, pointPos};
            }

            BeginCubemapShadowPass(pointLight->shadowID, CullMode::BACK);

            SetCamera(pointLight->cameraID);
            UpdateCamera(6, pointCamData);

            SetCubemapInfo(pointPos, pointLight->maxRange);

            startIndex = 0;
            for (std::pair<MeshID, u32> pair: meshCounts)
            {
                SetMesh(pair.first);
                DrawObjects(pair.second, startIndex);
                startIndex += pair.second;
            }
            EndPass();

            pointLightData.push_back({pointPos, pointLight->shadowID,
                                      pointLight->diffuse, pointLight->specular,
                                      pointLight->constant, pointLight->linear, pointLight->quadratic,
                                      pointLight->maxRange});
        }

        BeginDepthPass(CullMode::BACK);

        SetCamera(mainCam);
        CameraData mainCamData = {view, proj, cameraTransform->position};
        UpdateCamera(1, &mainCamData);

        startIndex = 0;
        for (std::pair<MeshID, u32> pair: meshCounts)
        {
            SetMesh(pair.first);
            DrawObjects(pair.second, startIndex);
            startIndex += pair.second;
        }
        EndPass();

        BeginColorPass(CullMode::BACK);

        SetLights(ambientLight,
                  dirLightData.size(), dirLightData.data(), cascades.data(),
                  spotLightData.size(), spotLightData.data(),
                  pointLightData.size(), pointLightData.data());

        startIndex = 0;
        for (std::pair<MeshID, u32> pair: meshCounts)
        {
            SetMesh(pair.first);
            DrawObjects(pair.second, startIndex);
            startIndex += pair.second;
        }
    #if SKL_ENABLED_EDITOR
        DrawImGui();
    #endif
        EndPass();
        EndFrame();
    }
};

class MovementSystem : public System
{
    void OnUpdate(Scene *scene, f32 deltaTime)
    {
        for (EntityID ent: SceneView<FlyingMovement, Transform3D>(*scene))
        {
            FlyingMovement *f = scene->Get<FlyingMovement>(ent);
            Transform3D *t = scene->Get<Transform3D>(ent);

            t->rotation.z += mouseDeltaX * f->turnSpeed;
            t->rotation.y += mouseDeltaY * f->turnSpeed;
            t->rotation.y = std::min(std::max(t->rotation.y, -90.0f), 90.0f);

            if (keysDown["W"])
            {
                t->position += GetForwardVector(t) * f->moveSpeed * deltaTime;
            }

            if (keysDown["S"])
            {
                t->position -= GetForwardVector(t) * f->moveSpeed * deltaTime;
            }

            if (keysDown["D"])
            {
                t->position += GetRightVector(t) * f->moveSpeed * deltaTime;
            }

            if (keysDown["A"])
            {
                t->position -= GetRightVector(t) * f->moveSpeed * deltaTime;
            }
        }
    }
};


// A vocabulary
//
// - EC: Road systems with extra credit
//
// A set of production rules
//
// - Subdivide: 5 -> 55
// - Plane Extend: 5 -> 5
//   - Rotate (inscribe)
//   - Place cuboid                - DONE
//   - Place trapezoid             - DONE
// - Pyramid Roof: 5 -> (6 | T)
// - Prism Roof: 5 -> T
// - Antenna: 6 -> T               - DONE
//
// An “axiom” (i.e. start state)
// 5
// A flat 2D plane that spans our entire city.

class BuilderSystem : public System
{
private:
    bool slowStep = false;
    f32 timer = 2.0f; // Seconds until next step
    f32 rate = 0.5f;   // Steps per second

    u32 pointLightCount = 0;
public:
    BuilderSystem(bool slowStep)
    {
        this->slowStep = slowStep;
    }

    void OnUpdate(Scene *scene, f32 deltaTime)
    {
        if (slowStep && timer > 0.0f)
        {
            timer -= deltaTime;
        }
        else
        {
            timer = 1.0f / rate;
            Step(scene);
        }
    }

    constexpr static f32 antennaHeightMin = 128;
    constexpr static f32 antennaHeightMax = 192;
    constexpr static f32 antennaWidth = 8;

    constexpr static f32 cuboidHeightMin = 32;
    constexpr static f32 cuboidHeightMax = 96;

    constexpr static f32 trapHeightMin = 24;
    constexpr static f32 trapHeightMax = 48;

    constexpr static f32 roofHeightMin = 32;
    constexpr static f32 roofHeightMax = 64;

    void Step(Scene *scene)
    {
        // Plane Rules
        for (EntityID ent: SceneView<Plane, Transform3D>(*scene))
        {
            Transform3D *t = scene->Get<Transform3D>(ent);
            Plane *plane = scene->Get<Plane>(ent);

            if (plane->width <= 16.0f || plane->length <= 16.0f || (plane->width / plane->length) >= 128 || (plane->length / plane->width) >= 128)
            {
                if (RandInBetween(0.0f, 1.0f) > 0.9375f)
                {
                    // Build antenna
                    f32 antennaHeight = RandInBetween(antennaHeightMin, antennaHeightMax);
                    BuildPart(scene, ent, t, cuboidMesh, {antennaWidth, antennaWidth, antennaHeight});
                    t->position.z -= antennaWidth / 2;

                    if (pointLightCount < 256)
                    {
                        EntityID pointLight = scene->NewEntity();
                        Transform3D* pointTransform = scene->Assign<Transform3D>(pointLight);
                        *pointTransform = *t;
                        pointTransform->position.z += antennaHeight / 2;
                        PointLight* pointLightComponent = scene->Assign<PointLight>(pointLight);
                        f32 red = RandInBetween(0.8, 1.0);
                        pointLightComponent->diffuse = {red, 0.6, 0.25};
                        pointLightComponent->specular = {red, 0.6, 0.25};
                        pointLightComponent->constant = 1;
                        pointLightComponent->linear = 0.0005;
                        pointLightComponent->quadratic = 0.00005;
                        pointLightComponent->maxRange = 1000;

                        pointLightCount++;

                        std::cout << pointLightCount << "\n";
                    }
                }

                scene->Remove<Plane>(ent);
                continue;
            }

            switch (RandInt(0, 13))
            {
            case 0:
                {
                    // Rotate
                    f32 shortSide = std::min(plane->width, plane->length);
                    f32 longSide = std::max(plane->width, plane->length);

                    f32 maxAngle = atan2(shortSide, longSide) - 0.02f;

                    f32 angle = RandInBetween(glm::radians(7.5f), maxAngle);

                    f32 costheta = cos(angle);
                    f32 sintheta = sin(angle);
                    f32 denom = ((costheta * costheta) - (sintheta * sintheta));
                    f32 width = ((plane->width * costheta) -
                                 (plane->length * sintheta)) / denom;
                    f32 length = ((plane->length * costheta) -
                                  (plane->width * sintheta)) / denom;
                    plane->width = width;
                    plane->length = length;

                    t->rotation.z += glm::degrees(angle);
                    break;
                }
            case 1:
                {
                    // Build Trapezoid
                    if (plane->width > 256 || plane->length > 256)
                    {
                        continue;
                    }

                    f32 trapHeight = RandInBetween(trapHeightMin, trapHeightMax);
                    BuildPart(scene, ent, t, trapMesh, {plane->length, plane->width, trapHeight});

                    EntityID newPlane = scene->NewEntity();
                    Transform3D *newT = scene->Assign<Transform3D>(newPlane);
                    Plane *p = scene->Assign<Plane>(newPlane);
                    *newT = *t;
                    newT->position.z += trapHeight / 2;
                    p->width = plane->width / 2;
                    p->length = plane->length / 2;

                    scene->Remove<Plane>(ent);
                    break;
                }
            case 2:
                {
                    // Build Pyramid Roof
                    if (plane->width > 96 || plane->length > 96)
                    {
                        continue;
                    }

                    f32 pyraHeight = RandInBetween(roofHeightMin, roofHeightMax);
                    BuildPart(scene, ent, t, pyraMesh, {plane->length, plane->width, pyraHeight});

                    scene->Remove<Plane>(ent);
                    break;
                }
            case 3:
                {
                    // Build Prism Roof
                    if (plane->width > 96)
                    {
                        continue;
                    }

                    f32 prismHeight = RandInBetween(roofHeightMin, roofHeightMax);
                    BuildPart(scene, ent, t, prismMesh, {plane->length, plane->width, prismHeight});

                    scene->Remove<Plane>(ent);
                    break;
                }
            case 4:
            case 5:
            case 6:
            case 7:
                {
                    // Build Cuboid
                    f32 cuboidHeight = RandInBetween(cuboidHeightMin, cuboidHeightMax);
                    BuildPart(scene, ent, t, cuboidMesh, {plane->length, plane->width, cuboidHeight});

                    EntityID newPlane = scene->NewEntity();
                    Transform3D *newT = scene->Assign<Transform3D>(newPlane);
                    Plane *p = scene->Assign<Plane>(newPlane);
                    *newT = *t;
                    newT->position.z += cuboidHeight / 2;
                    *p = *plane;

                    scene->Remove<Plane>(ent);
                    break;
                }
            default:
                {
                    // Subdivide
                    EntityID newPlane = scene->NewEntity();
                    Transform3D *newT = scene->Assign<Transform3D>(newPlane);
                    Plane *p = scene->Assign<Plane>(newPlane);
                    *newT = *t;
                    *p = *plane;

                    f32 ratio = RandInBetween(0.2f, 0.8f);

                    if (RandInBetween(0.0f, plane->width + plane->length) < plane->length)
                    {
                        // Split X axis
                        f32 old = plane->length;
                        f32 divisible = plane->length - 16.0f;

                        plane->length = divisible * ratio;
                        p->length = divisible * (1.0f - ratio);

                        t->position -= GetForwardVector(t) * ((old - plane->length) * 0.5f);
                        newT->position += GetForwardVector(newT) * ((old - p->length) * 0.5f);
                    }
                    else
                    {
                        // Split Y axis
                        f32 old = plane->width;
                        f32 divisible = plane->width - 16.0f;

                        plane->width = divisible * ratio;
                        p->width = divisible * (1.0f - ratio);

                        t->position -= GetRightVector(t) * ((old - plane->width) * 0.5f);
                        newT->position += GetRightVector(newT) * ((old - p->width) * 0.5f);
                    }
                }
            }
        }
    }

    void BuildPart(Scene *scene, EntityID ent, Transform3D *t, uint32_t mesh, glm::vec3 scale)
    {
        t->position.z += scale.z / 2;
        t->scale = scale;

        MeshComponent *m = scene->Assign<MeshComponent>(ent);
        m->mesh = mesh;
        ColorComponent *c = scene->Assign<ColorComponent>(ent);
        f32 shade = RandInBetween(0.25f, 0.75f);
        c->r = shade;
        c->g = shade;
        c->b = shade;
    }
};
