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

class RenderSystem : public System
{
    Mesh* currentMesh = nullptr;

    void OnUpdate(Scene *scene, f32 deltaTime)
    {
        if (!InitFrame())
        {
            return;
        }

        SceneView<CameraComponent, Transform3D> cameraView = SceneView<CameraComponent, Transform3D>(*scene);
        if (cameraView.begin() == cameraView.end())
        {
            return;
        }

        EntityID cameraEnt = *cameraView.begin();
        CameraComponent *camera = scene->Get<CameraComponent>(cameraEnt);
        Transform3D *cameraTransform = scene->Get<Transform3D>(cameraEnt);
        glm::mat4 view = GetViewMatrix(cameraTransform);
        f32 aspect = (f32)windowWidth / (f32)windowHeight;
        glm::mat4 proj = glm::perspective(glm::radians(camera->fov), aspect, camera->near, camera->far);

        SetCamera(view, proj, cameraTransform->position);

        // 1. Gather counts of each unique mesh pointer.
        std::map<Mesh *, u32> meshCounts;
        for (EntityID ent: SceneView<MeshComponent, Transform3D>(*scene))
        {
            MeshComponent *m = scene->Get<MeshComponent>(ent);
						++meshCounts[m->mesh];  // TODO: Verify the legitness of this
        }

        // 2. Create, with fixed size, the list of Mat4s, by adding up all of the counts.
        // 3. Get pointers to the start of each segment of unique mesh pointer.
        u32 totalCount = 0;
        std::unordered_map<Mesh *, u32> offsets;
        for (std::pair<Mesh *, u32> pair: meshCounts)
        {
            offsets[pair.first] = totalCount;
            totalCount += pair.second;
        }


        std::vector<glm::mat4> matrices(totalCount);


        // 4. Iterate through scene view once more and fill in the fixed size array.
        for (EntityID ent: SceneView<MeshComponent, Transform3D>(*scene))
        {
            Transform3D *t = scene->Get<Transform3D>(ent);
            glm::mat4 model = GetTransformMatrix(t);
            MeshComponent *m = scene->Get<MeshComponent>(ent);
            Mesh *mesh = m->mesh;

            matrices[offsets[mesh]++] = model;
        }

        SendModelMatrices(matrices);

        int startIndex = 0;
        for (std::pair<Mesh *, u32> pair: meshCounts)
        {
            SetMesh(pair.first);
            DrawObjects(pair.second, startIndex);
            startIndex += pair.second;
        }

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

            t->rotation.z += mouseRelX * f->turnSpeed;
            t->rotation.y += mouseRelY * f->turnSpeed;
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
    f32 timer = 0.0f; // Seconds until next step
    f32 rate = 0.5;   // Steps per second

    void OnUpdate(Scene *scene, f32 deltaTime)
    {
        if (timer > 0.0f)
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

    void Step(Scene* scene)
    {
        // Point Rules
        for (EntityID ent: SceneView<Point, Transform3D>(*scene))
        {
            Transform3D *t = scene->Get<Transform3D>(ent);

            f32 antennaHeight = RandInBetween(antennaHeightMin, antennaHeightMax);
            t->position.z += (antennaHeight / 2) - (antennaWidth / 2);
            t->scale.z = antennaHeight;
            t->scale.x = antennaWidth;
            t->scale.y = antennaWidth;


            MeshComponent *m = scene->Assign<MeshComponent>(ent);
            m->mesh = cuboidMesh;

            scene->Remove<Point>(ent);
        }

        // Plane Rules
        for (EntityID ent: SceneView<Plane, Transform3D>(*scene))
        {
            Transform3D *t = scene->Get<Transform3D>(ent);
            Plane *plane = scene->Get<Plane>(ent);

            switch (RandInt(0, 5))
            {
            case 0:
                {

                }
            case 1:
                {

                }
            case 2:
                {
                    f32 cuboidHeight = RandInBetween(cuboidHeightMin, cuboidHeightMax);
                    t->position.z += cuboidHeight / 2;
                    t->scale.z = cuboidHeight;
                    t->scale.x = plane->length;
                    t->scale.y = plane->width;

                    MeshComponent *m = scene->Assign<MeshComponent>(ent);
                    m->mesh = cuboidMesh;

                    EntityID newPlane = scene->NewEntity();
                    Transform3D *newT = scene->Assign<Transform3D>(newPlane);
                    Plane *p = scene->Assign<Plane>(newPlane);
                    *newT = *t;
                    newT->position += cuboidHeight / 2;
                    *p = *plane;

                    scene->Remove<Plane>(ent);
                    break;
                }
            case 3:
                {
                    f32 trapHeight = RandInBetween(trapHeightMin, trapHeightMax);
                    t->position.z += trapHeight / 2;
                    t->scale.z = trapHeight;
                    t->scale.x = plane->length;
                    t->scale.y = plane->width;

                    MeshComponent *m = scene->Assign<MeshComponent>(ent);
                    m->mesh = trapMesh;

                    EntityID newPlane = scene->NewEntity();
                    Transform3D *newT = scene->Assign<Transform3D>(newPlane);
                    Plane *p = scene->Assign<Plane>(newPlane);
                    *newT = *t;
                    newT->position += trapHeight / 2;
                    *p = *plane;

                    scene->Remove<Plane>(ent);
                    break;
                }
            case 4:
                {

                }
            case 5:
                {

                }
            }
        }
    }
};
