const vec3 sunLight  = normalize( vec3(  0.4, 0.4,  0.48 ) );
const vec3 sunColour = vec3(1.0, .9, .83);
const float specular = 0.0;
const vec2 add = vec2(1.0, 0.0);
#define HASHSCALE1 .1031
#define HASHSCALE3 vec3(.1031, .1030, .0973)
#define HASHSCALE4 vec4(1031, .1030, .0973, .1099)

float Hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
vec2 Hash22(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * HASHSCALE3);
    p3 += dot(p3, p3.yzx+19.19);
    return fract((p3.xx+p3.yz)*p3.zy);

}

float Noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);

    float res = mix(mix( Hash12(p),          Hash12(p + add.xy),f.x),
                    mix( Hash12(p + add.yx), Hash12(p + add.xx),f.x),f.y);
    return res;
}

vec2 Noise2( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y * 57.0;
   vec2 res = mix(mix( Hash22(p),          Hash22(p + add.xy),f.x),
                  mix( Hash22(p + add.yx), Hash22(p + add.xx),f.x),f.y);
    return res;
}
//--------------------------------------------------------------------------
// Low def version for ray-marching through the height field...
// Thanks to IQ for all the noise stuff...

float Terrain( in vec2 p)
{
	vec2 pos = p*0.05;
	float w = (Noise(pos*.25)*0.75+.15);
	w = 66.0 * w * w;
	vec2 dxy = vec2(0.0, 0.0);
	float f = .0;
	for (int i = 0; i < 5; i++)
	{
		f += w * Noise(pos);
		w = -w * 0.4;	//...Flip negative and positive for variation
	}
	float ff = Noise(pos*.002);

	f += pow(abs(ff), 5.0)*275.-5.0;
	return f;
}

vec3 CalculateNormal(vec2 pos)
{
    float heightLeft = Terrain(vec2(pos.x - 1, pos.y));
    float heightRight = Terrain(vec2(pos.x + 1., pos.y));
    float heightUp = Terrain(vec2(pos.x, pos.y + 1.));
    float heightDown = Terrain(vec2(pos.x, pos.y - 1.));

    // Calculate the two vectors on the surface
    vec3 v1 = vec3(2.0f, heightRight - heightLeft, 0.0f);  // Horizontal vector (x-direction)
    vec3 v2 = vec3(0.0f, heightDown - heightUp, 2.0f);    // Vertical vector (z-direction)

    // Compute the cross product to get the normal vector
    return normalize(cross(v1, v2));
}
