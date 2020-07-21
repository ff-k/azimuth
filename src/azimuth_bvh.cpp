static b32
ConstructBVH(entity **BVHNode, 
             entity *Entities, u32 EntityCount, u32 Depth=0){
    b32 Result = true;

    Assert(EntityCount != 0);

    if(EntityCount == 1){
        *BVHNode = Entities;
    } else {
        if(MemoryAllocate((void **)BVHNode, sizeof(entity))){
            (*BVHNode)->Type = EntityType_BVHNode;
            (*BVHNode)->Right = 0;
            (*BVHNode)->Left = 0;
            
            aabb AABB = { Vec3(f32_Max, f32_Max, f32_Max), 
                          Vec3(f32_Min, f32_Min, f32_Min) };
            for(u32 EntityIndex = 0;
                    EntityIndex < EntityCount;
                    EntityIndex++){
                entity *E = Entities + EntityIndex;

                switch(E->Type){
                    case EntityType_Sphere: {
                        vec3 Min = E->Center - E->Radius;
                        vec3 Max = E->Center + E->Radius;

                        vec4 Corners[] = {
                            Vec4(Min.x, Min.y, Min.z, 1.0f),
                            Vec4(Min.x, Min.y, Max.z, 1.0f),
                            Vec4(Min.x, Max.y, Min.z, 1.0f),
                            Vec4(Min.x, Max.y, Max.z, 1.0f),
                            Vec4(Max.x, Min.y, Min.z, 1.0f),
                            Vec4(Max.x, Min.y, Max.z, 1.0f),
                            Vec4(Max.x, Max.y, Min.z, 1.0f),
                            Vec4(Max.x, Max.y, Max.z, 1.0f)
                        };

                        for(u32 CornerIdx=0;
                                CornerIdx<8;
                                CornerIdx++){
                            vec4 V = Corners[CornerIdx];
                            vec4 TV = E->TransformationMatrix * V;

                            Assert(TV.w > 0.999f && TV.w < 1.001f);

                            if(Min.x > TV.x){ Min.x = TV.x; }
                            if(Min.y > TV.y){ Min.y = TV.y; }
                            if(Min.z > TV.z){ Min.z = TV.z; }
                            if(Max.x < TV.x){ Max.x = TV.x; }
                            if(Max.y < TV.y){ Max.y = TV.y; }
                            if(Max.z < TV.z){ Max.z = TV.z; }
                        }

                        if(Length(E->MotionVector) > 0.0f){
                            vec3 FinalMin = Min + E->MotionVector;
                            vec3 FinalMax = Max + E->MotionVector;
                            
                            if(Min.x > FinalMin.x){ Min.x = FinalMin.x; }
                            if(Min.y > FinalMin.y){ Min.y = FinalMin.y; }
                            if(Min.z > FinalMin.z){ Min.z = FinalMin.z; }

                            if(Max.x < FinalMax.x){ Max.x = FinalMax.x; }
                            if(Max.y < FinalMax.y){ Max.y = FinalMax.y; }
                            if(Max.z < FinalMax.z){ Max.z = FinalMax.z; }
                        }

                        if(AABB.Min.x > Min.x){ AABB.Min.x = Min.x; }
                        if(AABB.Min.y > Min.y){ AABB.Min.y = Min.y; }
                        if(AABB.Min.z > Min.z){ AABB.Min.z = Min.z; }
                        if(AABB.Max.x < Min.x){ AABB.Max.x = Min.x; }
                        if(AABB.Max.y < Min.y){ AABB.Max.y = Min.y; }
                        if(AABB.Max.z < Min.z){ AABB.Max.z = Min.z; }

                        if(AABB.Min.x > Max.x){ AABB.Min.x = Max.x; }
                        if(AABB.Min.y > Max.y){ AABB.Min.y = Max.y; }
                        if(AABB.Min.z > Max.z){ AABB.Min.z = Max.z; }
                        if(AABB.Max.x < Max.x){ AABB.Max.x = Max.x; }
                        if(AABB.Max.y < Max.y){ AABB.Max.y = Max.y; }
                        if(AABB.Max.z < Max.z){ AABB.Max.z = Max.z; }
                    } break;
                    case EntityType_Triangle: {
                        vec3 P0 = E->P0;
                        vec3 P1 = E->P1;
                        vec3 P2 = E->P2;

                        if(AABB.Min.x > P0.x){ AABB.Min.x = P0.x; }
                        if(AABB.Min.y > P0.y){ AABB.Min.y = P0.y; }
                        if(AABB.Min.z > P0.z){ AABB.Min.z = P0.z; }
                        if(AABB.Max.x < P0.x){ AABB.Max.x = P0.x; }
                        if(AABB.Max.y < P0.y){ AABB.Max.y = P0.y; }
                        if(AABB.Max.z < P0.z){ AABB.Max.z = P0.z; }

                        if(AABB.Min.x > P1.x){ AABB.Min.x = P1.x; }
                        if(AABB.Min.y > P1.y){ AABB.Min.y = P1.y; }
                        if(AABB.Min.z > P1.z){ AABB.Min.z = P1.z; }
                        if(AABB.Max.x < P1.x){ AABB.Max.x = P1.x; }
                        if(AABB.Max.y < P1.y){ AABB.Max.y = P1.y; }
                        if(AABB.Max.z < P1.z){ AABB.Max.z = P1.z; }

                        if(AABB.Min.x > P2.x){ AABB.Min.x = P2.x; }
                        if(AABB.Min.y > P2.y){ AABB.Min.y = P2.y; }
                        if(AABB.Min.z > P2.z){ AABB.Min.z = P2.z; }
                        if(AABB.Max.x < P2.x){ AABB.Max.x = P2.x; }
                        if(AABB.Max.y < P2.y){ AABB.Max.y = P2.y; }
                        if(AABB.Max.z < P2.z){ AABB.Max.z = P2.z; }
                    } break;
                    case EntityType_MeshInstance: {
                        vec3 Min = E->AABB.Min;
                        vec3 Max = E->AABB.Max;

                        if(AABB.Min.x > Min.x){ AABB.Min.x = Min.x; }
                        if(AABB.Min.y > Min.y){ AABB.Min.y = Min.y; }
                        if(AABB.Min.z > Min.z){ AABB.Min.z = Min.z; }
                        if(AABB.Max.x < Min.x){ AABB.Max.x = Min.x; }
                        if(AABB.Max.y < Min.y){ AABB.Max.y = Min.y; }
                        if(AABB.Max.z < Min.z){ AABB.Max.z = Min.z; }

                        if(AABB.Min.x > Max.x){ AABB.Min.x = Max.x; }
                        if(AABB.Min.y > Max.y){ AABB.Min.y = Max.y; }
                        if(AABB.Min.z > Max.z){ AABB.Min.z = Max.z; }
                        if(AABB.Max.x < Max.x){ AABB.Max.x = Max.x; }
                        if(AABB.Max.y < Max.y){ AABB.Max.y = Max.y; }
                        if(AABB.Max.z < Max.z){ AABB.Max.z = Max.z; }
                    } break;
                    case EntityType_BVHNode: {
                        vec3 Min = E->AABB.Min;
                        vec3 Max = E->AABB.Max;

                        if(AABB.Min.x > Min.x){ AABB.Min.x = Min.x; }
                        if(AABB.Min.y > Min.y){ AABB.Min.y = Min.y; }
                        if(AABB.Min.z > Min.z){ AABB.Min.z = Min.z; }
                        if(AABB.Max.x < Min.x){ AABB.Max.x = Min.x; }
                        if(AABB.Max.y < Min.y){ AABB.Max.y = Min.y; }
                        if(AABB.Max.z < Min.z){ AABB.Max.z = Min.z; }

                        if(AABB.Min.x > Max.x){ AABB.Min.x = Max.x; }
                        if(AABB.Min.y > Max.y){ AABB.Min.y = Max.y; }
                        if(AABB.Min.z > Max.z){ AABB.Min.z = Max.z; }
                        if(AABB.Max.x < Max.x){ AABB.Max.x = Max.x; }
                        if(AABB.Max.y < Max.y){ AABB.Max.y = Max.y; }
                        if(AABB.Max.z < Max.z){ AABB.Max.z = Max.z; }
                    } break;
                    UnexpectedDefaultCase;
                }
            }
            AABB.Center = AABB.Min*0.5f + AABB.Max*0.5f;
            (*BVHNode)->AABB = AABB;

            // TODO(furkan): Try sorting entities and divide 
            // them by half and compare results
            u32 Dim = Depth%3;
            f32 SplitAt = *(((f32 *)&AABB.Center) + Dim); 

            u32 EntityCountLeft = 0;
            for(u32 EntityIndex = 1;
                    EntityIndex < EntityCount;
                    EntityIndex++){
                entity *E = Entities + EntityIndex;

                f32 Pos = 0.0f;

                switch(E->Type){
                    case EntityType_Sphere: {
                        vec3 Center = PointToWorld(E->Center,
                                                   E->TransformationMatrix);
                        Pos = *(((f32 *)&Center) + Dim);
                    } break;
                    case EntityType_Triangle: {
                        Pos = *(((f32 *)&E->P0) + Dim);
                    } break;
                    case EntityType_MeshInstance: {
                        Pos = *(((f32 *)&E->AABB.Center) + Dim);
                    } break;
                    case EntityType_BVHNode: {
                        Pos = *(((f32 *)&E->AABB.Center) + Dim);
                    } break;
                    UnexpectedDefaultCase;
                }

                if(Pos < SplitAt){
                    entity SwapTemp = Entities[EntityCountLeft];
                    Entities[EntityCountLeft] = *E;
                    *E = SwapTemp;

                    EntityCountLeft++;
                }
            }
            if(EntityCountLeft == 0 || EntityCountLeft == EntityCount){
                // NOTE(furkan): Divide them by half if 
                // they are all on the exact same point
                EntityCountLeft = EntityCount/2;
            }

            Log("(%2u) ECL: %u, ECR: %u", Depth, EntityCountLeft, 
                                          (EntityCount-EntityCountLeft));

            u32 NextDepth = Depth + 1;
            if(!ConstructBVH(&(*BVHNode)->Left, 
                             Entities, 
                             EntityCountLeft, 
                             NextDepth)){
                Error("BVH construction has failed");
                Result = false;
            }

            if(!ConstructBVH(&(*BVHNode)->Right, 
                             Entities + EntityCountLeft, 
                             EntityCount - EntityCountLeft, 
                             NextDepth)){
                Error("BVH construction has failed");
                Result = false;
            }
        } else {
            Error("Memory allocation failed");
            Result = false;
        }
    }

    return Result;
}
                          
static b32
ConstructBVHTrianglesOnly(entity **BVHNode, 
                          entity *Triangles, u32 TriangleCount, u32 Depth=0){
    b32 Result = true;

    // NOTE(furkan): This function is doing the exact same thing with 
    // ConstructBVH function. The only difference is that it assumes 
    // all of the entities are triangles. So, it does not check 
    // entity type for every single triangle.

    Assert(TriangleCount != 0);

    if(TriangleCount == 1){
        *BVHNode = Triangles;
    } else {
        if(MemoryAllocate((void **)BVHNode, sizeof(entity))){
            (*BVHNode)->Type = EntityType_BVHNode;
            (*BVHNode)->Right = 0;
            (*BVHNode)->Left = 0;
            
            aabb AABB = { Vec3(f32_Max, f32_Max, f32_Max), 
                          Vec3(f32_Min, f32_Min, f32_Min) };
            for(u32 TriangleIndex = 0;
                    TriangleIndex < TriangleCount;
                    TriangleIndex++){
                entity *T = Triangles + TriangleIndex;
                Assert(T->Type == EntityType_Triangle);

                vec3 P0 = T->P0;
                vec3 P1 = T->P1;
                vec3 P2 = T->P2;

                if(AABB.Min.x > P0.x){ AABB.Min.x = P0.x; }
                if(AABB.Min.y > P0.y){ AABB.Min.y = P0.y; }
                if(AABB.Min.z > P0.z){ AABB.Min.z = P0.z; }
                if(AABB.Max.x < P0.x){ AABB.Max.x = P0.x; }
                if(AABB.Max.y < P0.y){ AABB.Max.y = P0.y; }
                if(AABB.Max.z < P0.z){ AABB.Max.z = P0.z; }

                if(AABB.Min.x > P1.x){ AABB.Min.x = P1.x; }
                if(AABB.Min.y > P1.y){ AABB.Min.y = P1.y; }
                if(AABB.Min.z > P1.z){ AABB.Min.z = P1.z; }
                if(AABB.Max.x < P1.x){ AABB.Max.x = P1.x; }
                if(AABB.Max.y < P1.y){ AABB.Max.y = P1.y; }
                if(AABB.Max.z < P1.z){ AABB.Max.z = P1.z; }

                if(AABB.Min.x > P2.x){ AABB.Min.x = P2.x; }
                if(AABB.Min.y > P2.y){ AABB.Min.y = P2.y; }
                if(AABB.Min.z > P2.z){ AABB.Min.z = P2.z; }
                if(AABB.Max.x < P2.x){ AABB.Max.x = P2.x; }
                if(AABB.Max.y < P2.y){ AABB.Max.y = P2.y; }
                if(AABB.Max.z < P2.z){ AABB.Max.z = P2.z; }
            }
            AABB.Center = AABB.Min*0.5f + AABB.Max*0.5f;
            (*BVHNode)->AABB = AABB;

            u32 Dim = Depth%3;
            f32 SplitAt = *(((f32 *)&AABB.Center) + Dim); 

            u32 TriangleCountLeft = 0;
            for(u32 TriangleIndex = 1;
                    TriangleIndex < TriangleCount;
                    TriangleIndex++){
                entity *T = Triangles + TriangleIndex;
                f32 Pos = *(((f32 *)&T->P0) + Dim);
                if(Pos < SplitAt){
                    entity SwapTemp = Triangles[TriangleCountLeft];
                    Triangles[TriangleCountLeft] = *T;
                    *T = SwapTemp;

                    TriangleCountLeft++;
                }
            }
            if(TriangleCountLeft == 0 || TriangleCountLeft == TriangleCount){
                // NOTE(furkan): Divide them by half if 
                // they are all on the exact same point
                TriangleCountLeft = TriangleCount/2;
            }

            Log("(%2u) TCL: %u, TCR: %u", Depth, TriangleCountLeft, 
                                          (TriangleCount-TriangleCountLeft));

            u32 NextDepth = Depth + 1;
            if(!ConstructBVHTrianglesOnly(&(*BVHNode)->Left, 
                                          Triangles, 
                                          TriangleCountLeft, 
                                          NextDepth)){
                Error("BVH construction has failed");
                Result = false;
            }

            if(!ConstructBVHTrianglesOnly(&(*BVHNode)->Right, 
                                          Triangles + TriangleCountLeft, 
                                          TriangleCount - TriangleCountLeft, 
                                          NextDepth)){
                Error("BVH construction has failed");
                Result = false;
            }
        } else {
            Error("Memory allocation failed");
            Result = false;
        }
    }

    return Result;
}
