struct PixelInput {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL;

	float3 lightDir : COLOR1;
};

struct Light
{
    float4 dir;
    float4 ambient;
    float4 diffuse;
};

cbuffer cbPerObject
{
    float4x4 WVP;
    float4x4 World;
	float4x4 View;
	float4x4 Proj;
};

cbuffer cbPerFrame
{
	Light light;
};

PixelInput VS_main( float4 pos : POSITION, float4 color : COLOR0, float3 normal : NORMAL)
{
	PixelInput o;
	matrix WV = mul(World, View);

	o.pos = mul(pos, WVP);
	o.normal = mul(normal, WV);
	o.color = color;

	float3 wvpos = mul(float4(pos.xyz, 1), WV).xyz;
	o.lightDir = mul(light. dir, View).xyz - wvpos;

	return o;
}

float4 PS_main( PixelInput i ) : SV_TARGET
{
	i.normal = normalize(i.normal);
	float L = saturate(dot(normalize(i.lightDir), i.normal));

	float4 diffuse = i.color;

    float3 finalColor;

    finalColor = diffuse * light.ambient;
    finalColor += diffuse * L * light.diffuse;

	return float4(finalColor, 1);
	//return float4(i.normal, 1);
}
