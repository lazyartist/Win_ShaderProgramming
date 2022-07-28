sampler2D DiffuseSampler;

struct PS_INPUT
{
   float2 mTexCoord : TEXCOORD0;
};

float4 ps_main(PS_INPUT Input) : COLOR
{
   float4 albedo = tex2D(DiffuseSampler, Input.mTexCoord);
   return  albedo;
}