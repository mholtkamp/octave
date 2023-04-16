
#define PATH_TRACE_LIGHT_POINT 0
#define PATH_TRACE_LIGHT_DIRECTIONAL 1

struct Ray
{
    vec3 mOrigin;
    vec3 mDirection;
};

struct HitInfo
{
    bool mHit;
    float mDistance;
    vec3 mPosition;
    vec3 mNormal;

    MaterialUniforms mMaterial;
};

struct PathTraceVertex
{
    vec3 mPosition;
    float mPad0;

    vec2 mTexcoord0;
    vec2 mTexcoord1;

    vec3 mNormal;
    uint mColor;
};

struct PathTraceTriangle
{
    PathTraceVertex mVertices[3];
};

struct PathTraceMesh
{
    vec4 mBounds;

    uint mStartTriangleIndex;
    uint mNumTriangles;
    uint mPad0;
    uint mPad1;
    
    MaterialUniforms mMaterial;
};

struct PathTraceLight
{
    vec3 mPosition;
    float mRadius;

    vec4 mColor;

    uint mLightType;
    vec3 mDirection;
};

struct PathTraceUniforms
{
    uint mNumTriangles;
    uint mNumMeshes;
    uint mNumLights;
    uint mMaxBounces;

    uint mRaysPerPixel;
    uint mAccumulatedFrames;
    uint mPad1;
    uint mPad2;
};

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

HitInfo RayTriangleTest(Ray ray, PathTraceTriangle tri)
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
    const vec3 kSkyHorizonColor = vec3(0.5, 0.5, 0.8);
    const vec3 kSkyZenithColor = vec3(0.0, 0.2, 0.6);
    const vec3 kGroundColor = vec3(0.3, 0.35, 0.32);

    float skyAlpha = pow(smoothstep(0, 0.4, ray.mDirection.y), 0.35);
    vec3 skyColor = mix(kSkyHorizonColor, kSkyZenithColor, skyAlpha);

    float groundAlpha = smoothstep(-0.01, 0.0, ray.mDirection.y);
    vec3 envColor = mix(kGroundColor, skyColor, groundAlpha);

    return envColor;
}
