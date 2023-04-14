
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
    uint mPad0;
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
