// GLOBALS
// ----------

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

Texture2D shaderTexture;
SamplerState SampleType;

// TYPEDEFS
// ----------

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

PixelInputType vxmain(VertexInputType i)
{
	PixelInputType o;
	// Change the position vector to be 4 units for proper matrix calculations.
    i.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    o.position = mul(input.position, worldMatrix);
    o.position = mul(output.position, viewMatrix);
    o.position = mul(output.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
    o.tex = i.tex;
    
    return o;
}

float4 pxmain(PixelInputType i) : SV_TARGET
{
    float4 textureColor;

    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = shaderTexture.Sample(SampleType, i.tex);

    return textureColor;
}
