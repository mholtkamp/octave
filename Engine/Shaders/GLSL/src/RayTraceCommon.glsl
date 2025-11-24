HitInfo CreateHitInfo()
{
    HitInfo hitInfo;
    hitInfo.mHit = false;
    hitInfo.mDistance = 0.0;
    hitInfo.mPosition = vec3(0,0,0);
    hitInfo.mNormal = vec3(0,0,0);

    return hitInfo;
}

float Rand(inout uint state)
{
    state = state * 747796405u + 2891336453u;
	uint result = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	result = (result >> 22u) ^ result;
    return result / 4294967295.0;
}

float RandomValueNormalDistribution(inout uint state)
{
    float theta = 2 * 3.1415926 * Rand(state);
    float rho = sqrt(-2 * log(Rand(state)));
    return rho * cos(theta);
}

vec3 RandomDirection(inout uint state)
{
    float x = RandomValueNormalDistribution(state);
    float y = RandomValueNormalDistribution(state);
    float z = RandomValueNormalDistribution(state);
    return normalize(vec3(x,y,z));
}

vec3 RandomHemisphereDirection(vec3 normal, inout uint state)
{
    vec3 dir = RandomDirection(state);
    return dir * sign(dot(normal, dir));
}

// https://www.rorydriscoll.com/2009/01/07/better-sampling/
vec3 CosineSampleHemisphere(float u1, float u2)
{
    const float r = sqrt(u1);
    const float theta = 2 * PI * u2;
 
    const float x = r * cos(theta);
    const float y = r * sin(theta);
 
    return vec3(x, y, sqrt(max(0.0, 1.0 - u1)));
}

void CreateCoordinateSystem(vec3 N, inout vec3 Nt, inout vec3 Nb) 
{ 
    if (abs(N.x) > abs(N.y))
        Nt = vec3(N.z, 0, -N.x) / sqrt(N.x * N.x + N.z * N.z);
    else
        Nt = vec3(0, -N.z, N.y) / sqrt(N.y * N.y + N.z * N.z);
    Nb = cross(N, Nt);
}

vec3 RandomHemisphereDirectionCos(vec3 normal, inout uint state)
{
    float u1 = Rand(state);
    float u2 = Rand(state);

    vec3 hemiPoint = CosineSampleHemisphere(u1, u2);

    vec3 tangent;
    vec3 bitangent;
    CreateCoordinateSystem(normal, tangent, bitangent);
    mat3 transMat = mat3(tangent, bitangent, normal);

    hemiPoint = transMat * hemiPoint;
    return normalize(hemiPoint);
}

HitInfo RayTriangleTest(Ray ray, RayTraceTriangle tri)
{
    vec3 edgeAB = tri.mVertices[1].mPosition - tri.mVertices[0].mPosition;
    vec3 edgeAC = tri.mVertices[2].mPosition - tri.mVertices[0].mPosition;
    vec3 normal = cross(edgeAB, edgeAC);
    vec3 ao = ray.mOrigin - tri.mVertices[0].mPosition;
    vec3 dao = cross(ao, ray.mDirection);

    float determinant = -dot(ray.mDirection, normal);
    float invDet = 1.0 / determinant;

    float dist = dot(ao, normal) * invDet;
    float u = dot(edgeAC, dao) * invDet;
    float v = -dot(edgeAB, dao) * invDet;
    float w = 1.0 - u - v;

    HitInfo hitInfo;
    hitInfo.mHit = determinant >= 1e-6 && dist >= 0.0 && u >= 0.0 && v >= 0.0 && w >= 0.0;
    hitInfo.mPosition = ray.mOrigin + ray.mDirection * dist;
    hitInfo.mNormal = normalize(tri.mVertices[0].mNormal * w + tri.mVertices[1].mNormal * u + tri.mVertices[2].mNormal * v);
    hitInfo.mUv0 = tri.mVertices[0].mTexcoord0 * w + tri.mVertices[1].mTexcoord0 * u + tri.mVertices[2].mTexcoord0 * v;
    hitInfo.mUv1 = tri.mVertices[0].mTexcoord1 * w + tri.mVertices[1].mTexcoord1 * u + tri.mVertices[2].mTexcoord1 * v;
    hitInfo.mColor = tri.mVertices[0].mColor * w + tri.mVertices[1].mColor * u + tri.mVertices[2].mColor * v;
    hitInfo.mDistance = dist;
    return hitInfo;
}

HitInfo RaySphereTest(Ray ray, vec3 sphereCenter, float sphereRadius)
{
    HitInfo hitInfo = CreateHitInfo();

    vec3 offsetRayOrigin = ray.mOrigin - sphereCenter;

    // Ray intersects sphere when the distance from the origin of a point along the ray 
    // is less than the radius of the sphere.
    // length(Pos + Dir * Distance)^2 == r^2
    float a = dot(ray.mDirection, ray.mDirection);
    float b = 2 * dot(offsetRayOrigin, ray.mDirection);
    float c = dot(offsetRayOrigin, offsetRayOrigin) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant >= 0)
    {
        float dist = (-b - sqrt(discriminant)) / (2 * a);

        if (dist >= 0)
        {
            hitInfo.mHit = true;
            hitInfo.mDistance = dist;
            hitInfo.mPosition = ray.mOrigin + ray.mDirection * dist;
            hitInfo.mNormal = normalize(hitInfo.mPosition - sphereCenter);
        }
    }

    return hitInfo;
}

bool RayOverlapsSphere(Ray ray, vec3 sphereCenter, float sphereRadius)
{
    bool insideSphere = false;
    bool intersectSphere = false;

    // Check if ray is inside sphere
    vec3 distVect = (ray.mOrigin - sphereCenter);
    float dist2 = dot(distVect, distVect);

    if (dist2 < sphereRadius * sphereRadius)
    {
        insideSphere = true;
    }

    if (!insideSphere)
    {
        HitInfo boundsHit = RaySphereTest(ray, sphereCenter, sphereRadius);
        intersectSphere = boundsHit.mHit;
    }

    return (insideSphere || intersectSphere);
}

vec3 GetEnvironmentLight(Ray ray)
{
    const vec3 kSkyHorizonColor = rayTrace.mSkyHorizonColor.rgb;
    const vec3 kSkyZenithColor = rayTrace.mSkyZenithColor.rgb;
    const vec3 kGroundColor = rayTrace.mGroundColor.rgb;

    float skyAlpha = pow(smoothstep(0, 0.4, ray.mDirection.y), 0.35);
    vec3 skyColor = mix(kSkyHorizonColor, kSkyZenithColor, skyAlpha);

    float groundAlpha = smoothstep(-0.01, 0.0, ray.mDirection.y);
    vec3 envColor = mix(kGroundColor, skyColor, groundAlpha);

    return envColor;
}


HitInfo CalculateRayCollision(Ray ray, bool shadowRay)
{
    // Trace bounding spheres of all meshes
    HitInfo closestHit = CreateHitInfo();
    closestHit.mDistance = 1e20;
    closestHit.mMesh.mMaterial.mColor = vec4(0,0,0,1);

    for (uint m = 0; m < rayTrace.mNumMeshes; ++m)
    {
        RayTraceMesh mesh = meshes[m];

        if (shadowRay && (mesh.mCastShadows == 0))
        {
            continue;
        }

        //HitInfo boundsHit = RaySphereTest(ray, mesh.mBounds.xyz, mesh.mBounds.w);
        bool overlapsBounds = RayOverlapsSphere(ray, mesh.mBounds.xyz, mesh.mBounds.w);

        if (overlapsBounds)
        {
            // Loop through triangles
            for (uint t = 0; t < mesh.mNumTriangles; ++t)
            {
                uint triIndex = mesh.mStartTriangleIndex + t;
                RayTraceTriangle tri = triangles[triIndex];
                HitInfo hitInfo = RayTriangleTest(ray, tri);

                if (hitInfo.mHit && hitInfo.mDistance < closestHit.mDistance)
                {
                    closestHit = hitInfo;
                    closestHit.mMesh = mesh;
                }
            }
        }
    }

    return closestHit;
} 

vec3 PathTrace(Ray ray, inout uint rngState)
{
    vec3 incomingLight = vec3(0,0,0);
    vec3 rayColor = vec3(1,1,1);

    const uint kMaxAlphaSkips = 5;
    uint maxBounces = rayTrace.mMaxBounces;
    uint numAlphaSkips = 0;

    for (int i = 0; i < maxBounces; ++i)
    {
        HitInfo hit = CalculateRayCollision(ray, false);
        if (hit.mHit)
        {
            RayTraceMesh mesh = hit.mMesh;
            MaterialUniforms material = mesh.mMaterial;

            bool transparent = (material.mBlendMode == BLEND_MODE_TRANSLUCENT || material.mBlendMode == BLEND_MODE_ADDITIVE);
            bool additive = (material.mBlendMode == BLEND_MODE_ADDITIVE);
            bool unlit = (material.mShadingModel == SHADING_MODEL_UNLIT);
            bool hasBakedLighting = (mesh.mHasBakedLighting != 0);
            
            vec4 surfaceColor = vec4(1,1,1,1);
            vec4 surfaceLitColor = vec4(0,0,0,0);
            vec2 uv0 = (hit.mUv0 + material.mUvOffset0) * material.mUvScale0;
            vec2 uv1 = (hit.mUv1 + material.mUvOffset1) * material.mUvScale1;

            surfaceColor = BlendTexture(material, surfaceColor, 0, textures[mesh.mTextures[0]], uv0, uv1, hit.mColor.r, material.mTevModes[0], material.mVertexColorMode);
            surfaceColor = BlendTexture(material, surfaceColor, 1, textures[mesh.mTextures[1]], uv0, uv1, hit.mColor.g, material.mTevModes[1], material.mVertexColorMode);
            surfaceColor = BlendTexture(material, surfaceColor, 2, textures[mesh.mTextures[2]], uv0, uv1, hit.mColor.b, material.mTevModes[2], material.mVertexColorMode);
            surfaceColor = BlendTexture(material, surfaceColor, 3, textures[mesh.mTextures[3]], uv0, uv1, 0.0, material.mTevModes[3], material.mVertexColorMode);

            surfaceColor *= material.mColor;

            surfaceLitColor = surfaceColor;
            
            if (material.mVertexColorMode == VERTEX_COLOR_MODULATE)
            {
                surfaceLitColor *= hit.mColor;
            }
            else if (material.mVertexColorMode == VERTEX_COLOR_TEXTURE_BLEND)
            {
                surfaceLitColor *= hit.mColor.a;
            }

            if (transparent)
            {
                surfaceLitColor.a *= material.mOpacity;
            }

            if (material.mBlendMode == BLEND_MODE_MASKED && surfaceColor.a < material.mMaskCutoff && numAlphaSkips < kMaxAlphaSkips)
            {
                // Instead of bouncing the ray, continue forward since it didn't really hit the surface.
                // Move the ray slightly pass the hit point so that we don't reintersect with the same triangle?
                ray.mOrigin = hit.mPosition + ray.mDirection * 0.01;
                --i;
                ++numAlphaSkips;
                continue;
            }

            // For now, only unlit objects can emit light.
            float emission = material.mEmission;
            if (/*unlit ||*/ hasBakedLighting)
            {
                emission = max(emission, 1.0);
            }
            
            vec3 emittedLight = emission * surfaceLitColor.rgb;

            if (transparent)
            {
                emittedLight = mix(vec3(0,0,0), emittedLight, surfaceLitColor.a);
            }

            incomingLight += (emittedLight * rayColor);

            if (transparent)
            {
                if (!additive)
                {
                    rayColor *= mix(vec3(1,1,1), surfaceColor.rgb, surfaceColor.a);
                }
            }
            else
            {
                rayColor *= surfaceColor.rgb;
            }

            // Determine new bounced ray direction.
            if (transparent && numAlphaSkips < kMaxAlphaSkips)
            {
                ray.mOrigin = hit.mPosition + ray.mDirection * 0.01;
                --i;
                ++numAlphaSkips;
            }
            else
            {
                ray.mOrigin = hit.mPosition + hit.mNormal * 0.01;
                vec3 diffuseDir = normalize(hit.mNormal + RandomDirection(rngState));
                vec3 specularDir = reflect(ray.mDirection, hit.mNormal);
                ray.mDirection = mix(diffuseDir, specularDir, material.mSpecular);
            }
        }
        else
        {
            incomingLight += GetEnvironmentLight(ray) * rayColor;
            break;
        }
    }

    return incomingLight;
}
