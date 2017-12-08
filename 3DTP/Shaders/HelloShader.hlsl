struct PixelInput {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

cbuffer cbPerObject
{
    float4x4 WVP;
};

PixelInput vxmain( float4 pos : POSITION, float4 color : COLOR0 )
{
	PixelInput o;
	o.pos = mul(pos, WVP);
	o.color = color;
	return o;
}

float4 pxmain( PixelInput i ) : SV_TARGET
{
	return i.color;
}
