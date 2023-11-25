// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);


cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
    float4 diffuseColor;
    float3 lightPosition;
    float padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;
};

float4 main(InputType input) : SV_TARGET
{
	float4	textureColor;
    float3	lightDir;
    float	lightIntensity;
    float4	color;
	float4 diffuse;

	// Invert the light direction for calculations.
	lightDir = normalize(input.position3D - lightPosition);

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, -lightDir));

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	// P	is the point in 3D space that we want to shade,
	// N	is the surface normal at point we want to shade,
	// Lp	is the position of the light in 3D space,
	// Ld	is the diffuse contribution of the light source,
	// L	is the normalized direction vector from the point we want to shade to the light source,
	// kd	is the diffuse component of the material,
	
	// Then L == normalize(Lp−P)
	// diffuse max(0,L⋅N)Ld.kd

	float3 dir = normalize(lightPosition - input.position3D);
	diffuse = float4(max(0, dir * input.normal)*diffuseColor,0);
	
	// color = ambientColor + (diffuse * lightIntensity); //adding ambient
	color = ambientColor + (diffuseColor * lightIntensity); //adding ambient
	color = saturate(color);


	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	if(textureColor.a < 1)
	{
		float averagecol = 1. - (textureColor.r+ textureColor.g + textureColor.b)/3.;
		float whitetoalpha = (averagecol) + textureColor.a;
		color = color * averagecol;
		color.a = whitetoalpha;
		// color.r = 1;
		return color;
	}
	color = color * textureColor;
	// color.a = 1;

    return color;
}

