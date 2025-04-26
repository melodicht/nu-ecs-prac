class GravitySystem : public System
{
    void OnUpdate(Scene *scene)
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
    void OnUpdate(Scene *scene)
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

    void OnUpdate(Scene *scene)
    {
        InitFrame();

        SceneView<CameraComponent, Transform3D> cameraView = SceneView<CameraComponent, Transform3D>(*scene);
        if (cameraView.begin() == cameraView.end())
        {
            return;
        }

        EntityID cameraEnt = *cameraView.begin();
        CameraComponent *camera = scene->Get<CameraComponent>(cameraEnt);
        Transform3D *cameraTransform = scene->Get<Transform3D>(cameraEnt);
        glm::mat4 view = GetViewMatrix(cameraTransform);
        f32 aspect = (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT;
        glm::mat4 proj = glm::perspective(glm::radians(camera->fov), aspect, camera->near, camera->far);

        SetCamera(view, proj);

        std::vector<glm::mat4> objects;

        for (EntityID ent: SceneView<MeshComponent, Transform3D>(*scene))
        {
            Transform3D *t = scene->Get<Transform3D>(ent);

            glm::mat4 model = GetTransformMatrix(t);

            objects.push_back(model);
        }

        SendModelMatrices(objects);

        int index = 0;
        for (EntityID ent: SceneView<Transform3D, MeshComponent>(*scene))
        {
            MeshComponent *meshc = scene->Get<MeshComponent>(ent);
            Mesh *mesh = meshc->mesh;
            if (currentMesh != mesh)
            {
                SetMesh(mesh);
                currentMesh = mesh;
            }

            DrawObject(index++);
        }
    }
};
