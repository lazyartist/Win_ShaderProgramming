//**************************************************************//
//  Effect File exported by RenderMonkey 1.6
//
//  - Although many improvements were made to RenderMonkey FX  
//    file export, there are still situations that may cause   
//    compilation problems once the file is exported, such as  
//    occasional naming conflicts for methods, since FX format 
//    does not support any notions of name spaces. You need to 
//    try to create workspaces in such a way as to minimize    
//    potential naming conflicts on export.                    
//    
//  - Note that to minimize resulting name collisions in the FX 
//    file, RenderMonkey will mangle names for passes, shaders  
//    and function names as necessary to reduce name conflicts. 
//**************************************************************//

//--------------------------------------------------------------//
// UVAnimation
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// Pass 0
//--------------------------------------------------------------//
string UVAnimation_Pass_0_Model : ModelData = "C:\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Models\\Torus.3ds";

float4x4 gWorldMatrix : World;
float4x4 gViewMatrix : View;
float4x4 gProjectionMatrix : Projection;

float4 gWorldLightPosition
<
   string UIName = "gWorldLightPosition";
   string UIWidget = "Direction";
   bool UIVisible =  false;
   float4 UIMin = float4( -10.00, -10.00, -10.00, -10.00 );
   float4 UIMax = float4( 10.00, 10.00, 10.00, 10.00 );
   bool Normalize =  false;
> = float4( 500.00, 500.00, -500.00, 1.00 );
float4 gWorldCameraPosition : ViewPosition;

float gTime : Time0_X;
float gWaveHeight
<
   string UIName = "gWaveHeight";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 0.00;
> = float( 5.27 );
float gSpeed
<
   string UIName = "gSpeed";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 0.00;
> = float( 2.00 );
float gWaveFrequency
<
   string UIName = "gWaveFrequency";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 0.00;
> = float( 46.59 );
float gUVSpeed
<
   string UIName = "gUVSpeed";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 0.00;
> = float( 0.14 );

struct VS_INPUT
{
   float4 mPosition : POSITION;
   float3 mNormal : NORMAL;
   float2 mUV : TEXCOORD0;
};

struct VS_OUTPUT
{
   float4 mPosition : POSITION;
   float2 mUV : TEXCOORD0;
   float3 mDiffuse : TEXCOORD1;
   float3 mViewDir : TEXCOORD2;
   float3 mReflection : TEXCOORD3;
};

// Default Model Orientation of the RenderMonkey is LHS(Left Hand Side) and Forward vector is z.
// UnrealEngine is LHS too, but forward vector is x.

VS_OUTPUT UVAnimation_Pass_0_Vertex_Shader_vs_main(VS_INPUT Input)
{
   VS_OUTPUT Output;
   
   float cosTime = gWaveHeight * cos(gTime * gSpeed + (Input.mUV.x + Input.mUV.y) * gWaveFrequency);
   //Input.mPosition.y += cosTime;
   Input.mPosition += float4(Input.mNormal * cosTime, 0.f);
   
   Output.mPosition = mul(Input.mPosition, gWorldMatrix);
   
   // Diffuse Light - Lambert Model
   // It's possible to do the reverse of subtraction. but fits the variable name.
   float3 lightDir = Output.mPosition.xyz - gWorldLightPosition.xyz;
   
   // Without normalization, difference in contrast is noticeable.
   lightDir = normalize(lightDir); 
   
   float3 viewDir = normalize(Output.mPosition.xyz - gWorldCameraPosition.xyz);
   Output.mViewDir = viewDir;
   
   Output.mPosition = mul(Output.mPosition, gViewMatrix);
   Output.mPosition = mul(Output.mPosition, gProjectionMatrix);
   
   // Normal of vertex is changed to world space.(it doesn't matter if the position changes as we only need the direction of the vertex)
   float3 worldNormal = mul(Input.mNormal, (float3x3)gWorldMatrix); 
   worldNormal = normalize(worldNormal);
   
   // Dot product of two vectors to get cosine value.
   Output.mDiffuse = dot(-lightDir, worldNormal);
   
   // Specular Light - Phong Model
   Output.mReflection = reflect(lightDir, worldNormal);
   
   Output.mUV = Input.mUV + float2(gTime * gUVSpeed, 0);
   
   return Output;
}



struct PS_INPUT
{
   float2 mUV : TEXCOORD0;
   float3 mDiffuse : TEXCOORD1;
   float3 mViewDir : TEXCOORD2;
   float3 mReflection : TEXCOORD3;
};

texture DiffuseMap_Tex
<
   string ResourceName = "C:\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Textures\\Fieldstone.tga";
>;
sampler2D DiffuseSampler = sampler_state
{
   Texture = (DiffuseMap_Tex);
};
texture SpecularMap_Tex
<
   string ResourceName = "..\\Resources\\Fieldstone_SM.tga";
>;
sampler2D SpecularSampler = sampler_state
{
   Texture = (SpecularMap_Tex);
};

float3 gWorldLightColor
<
   string UIName = "gWorldLightColor";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float3( 1.00, 1.00, 0.00 );

float4 UVAnimation_Pass_0_Pixel_Shader_ps_main(PS_INPUT Input) : COLOR
{
   float4 albedo = tex2D(DiffuseSampler, Input.mUV);
   float3 diffuse = gWorldLightColor * albedo.rgb * saturate(Input.mDiffuse);
   
   float3 reflection = normalize(Input.mReflection);//renormalize bad normals(normals are bad while passing the interpolator)
   float3 viewDir = normalize(Input.mViewDir);//renormalize bad normals(normals are bad while passing the interpolator)
   float3 specular = 0;
   if(diffuse.x > 0)//If there is diffuse light, it calculates specular light.
   {
      specular = saturate(dot(reflection, -viewDir));
      specular = pow(specular, 20.f);//As the power increases, the area of specular light becomes narrower.
      
      float4 specularIntensity = tex2D(SpecularSampler, Input.mUV);
      specular *= specularIntensity.rgb * gWorldLightColor;
   }
   
   float3 ambient = float3(0.1f, 0.1f, 0.1f) * albedo;
   
   return float4(ambient + diffuse + specular, 1);
}
//--------------------------------------------------------------//
// Technique Section for UVAnimation
//--------------------------------------------------------------//
technique UVAnimation
{
   pass Pass_0
   {
      VertexShader = compile vs_2_0 UVAnimation_Pass_0_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 UVAnimation_Pass_0_Pixel_Shader_ps_main();
   }

}

