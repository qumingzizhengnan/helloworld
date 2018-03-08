////////////���峣������///////////
//����任����ĳ�������
cbuffer MatrixBuffer{
	matrix World;
	matrix View;
	matrix Projection;
	float4 EyePosition;
};

//������Ϣ�ĳ�������
cbuffer MateriaBuffer{
	float4 MatAmbient;    //���ʶԻ�����ķ�����
	float4 MatDiffuse;    //���ʶ��������ķ�����
	float4 MatSpecular;   //���ʶԾ����ķ�����
	float  MatPower;      //���ʵľ���ⷴ��ϵ��
}

//��Դ�ĳ�������
cbuffer LightBuffer
{
	int    type;          //��Դ����
	float4 LightPosition; //��Դλ��
	float4 LightDirection;//��Դ����
	float4 LightAmbient;  //������ǿ��
	float4 LightDiffuse;  //�������ǿ��
	float4 LightSpecular; //�����ǿ��
	float  LightAtt0;     //����˥������
	float  LightAtt1;     //һ��˥������
	float  LightAtt2;     //����˥������
	float  LightAlpha;    //�۹����׶�Ƕ�
	float  LightBeta;     //�۹����׶�Ƕ�
	float  LightFallOff;  //�۹��˥��ϵ��
}

Texture2D Texture;       //�������

SamplerState Sampler     //���������
{
	Filter = MIN_MAG_MIP_LINEAR;   //�������Թ���
	AddressU = WRAP;              //ѰַģʽΪWRAP
	AddressV = WRAP;              //ѰַģʽΪWRAP
};

//////////////////////////////////////////////////////////////////////////
//��������ṹ
//////////////////////////////////////////////////////////////////////////
//������ɫ��������ṹ
struct VS_INPUT
{
	float4 Pos : POSITION;   //λ��
	float3 Norm: NORMAL;      //������
	float2 Tex : TEXCOORD0;  //�������꣬��һ����ά����
};

//������ɫ��������ṹ
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;    //λ��
	float2 Tex : TEXCOORD0;      //����
	float3 Norm: TEXCOORD1;       //������
	float4 ViewDirection: TEXCOORD2;//�ӵ㷽��
	float4 LightVector: TEXCOORD3;//�Ե��Դ�;۹����Ч
	//ǰ����������¼�����������������һ��������¼���վ���
};

//////////////////////////////////////////////////////////////////////////
// ���������ɫ��
//////////////////////////////////////////////////////////////////////////

//������ɫ��
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;              //����һ��VS_OUTPUT����
	output.Pos = mul(input.Pos, World);       //��input�����Ͻ�������任
	output.Pos = mul(output.Pos, View);       //���й۲�任
	output.Pos = mul(output.Pos, Projection); //����ͶӰ�任

	output.Norm = mul(input.Norm, (float3x3)World);  //���output�ķ�����
	output.Norm = normalize(output.Norm);              //�Է��������й�һ��

	float4 worldPosition = mul(input.Pos, World);          //��ȡ�������������
		output.ViewDirection = EyePosition - worldPosition;    //��ȡ�ӵ㷽��
	output.ViewDirection = normalize(output.ViewDirection);//���ӵ㷽���һ��

	output.LightVector = LightPosition - worldPosition;    //��ȡ���շ���
	output.LightVector = normalize(output.LightVector);    //�����շ����һ��
	output.LightVector.w = length(LightPosition - worldPosition);  //��ȡ���վ���

	output.Tex = input.Tex;//��������

	return output;
}

//������ɫ��
//�޹���������ɫ��
float4 PS(VS_OUTPUT input) : SV_Target
{
	return Texture.Sample(Sampler, input.Tex);//��������
}

// ƽ�й�Դ������ɫ��
// ������PS_INPUT�Ķ�����Ϊ����������һ��float4�ı�����Ϊ��ɫ
// ע�⣺�������ɫ��������ɫ�����������ɹ��պͲ����໥���ò�������ɫ
float4 PSDirectionalLight(VS_OUTPUT input) : SV_Target
{
	float4 finalColor;           //������ɫ�����������ɫ�����յ���ɫ
	float4 ambientColor, diffuseColor, specularColor; //���������⣬�����䣬�������ɫ

	//���շ���,�͹������䷽���෴
	float3 lightVector = -LightDirection.xyz;  //������.xyz�ķ�ʽȥfloat4�����ǰ3λ����
		lightVector = normalize(lightVector);      //��һ����������
	//�ò��ʻ����ⷴ���ʺͻ�����ǿ����˵õ���������ɫ
	ambientColor = MatAmbient * LightAmbient;
	//�����㷨�����͹��շ�����е�˵õ������������
	float diffuseFactor = dot(lightVector, input.Norm);
	if (diffuseFactor > 0.0f)  //�����������>0��ʾ���Ǳ�����
	{
		//�ò��ʵ��������ķ����ʺ��������Ĺ���ǿ���Լ��������������˵õ����������ɫ
		diffuseColor = MatDiffuse * LightDiffuse * diffuseFactor;

		//���ݹ��շ���Ͷ��㷨�������㷴�䷽��
		float3 reflection = reflect(-lightVector, input.Norm);
			//���ݷ��䷽���ӵ㷽���Լ����ʵľ���ⷴ��ϵ�������㾵�淴������
			//pow(b, n)����b��n�η�
			float specularFactor = pow(max(dot(reflection, input.ViewDirection.xyz), 0.0f), MatPower);
		//���ʵľ��淴���ʣ������ǿ���Լ����淴��������˵õ��������ɫ
		specularColor = MatSpecular * LightSpecular * specularFactor;
	}
	float4 texColor = float4(1.f, 1.f, 1.f, 1.f);
		texColor = Texture.Sample(Sampler, input.Tex);//��ȡ������ɫ
	//������ɫ�ɾ��⣬�����䣬�������ɫ������ӵõ�
	//saturate��ʾ���ʹ����������1�ͱ��1��С��0�ͱ��0���Ա�֤�����0-1֮��
	//��ȡ������ɫ
	finalColor = saturate(ambientColor + diffuseColor + specularColor);
	finalColor = texColor * finalColor;//������ɫ�������ɫ��˵õ�������ɫ
	finalColor.a = texColor.a * MatDiffuse.a;//��ȡalphaֵ��͸����
	return finalColor;
}

// ���Դ������ɫ��
// ���㷽���ͷ���ⲻͬ����Ҫ��������������ɫ�ļ��㹫ʽ��
float4 PSPointLight(VS_OUTPUT input) : SV_Target
{
	float4 finalColor;         //������ɫ�����������ɫ�����յ���ɫ
	float4 ambientColor, diffuseColor, specularColor;  //���������⣬�����䣬�������ɫ

	//�����������ڶ���Ĺ�������
	float3 lightVector = input.LightVector.xyz;
		lightVector = normalize(lightVector);   //��һ��
	//�ò��ʻ����ⷴ���ʺͻ�����ǿ����˵õ���������ɫ
	ambientColor = MatAmbient * LightAmbient;
	//�����㷨�����͹��շ�����е�˵õ������������
	float diffuseFactor = dot(lightVector, input.Norm);
	if (diffuseFactor > 0.0f)  //�����������>0��ʾ���Ǳ�����
	{
		//�ò��ʵ��������ķ����ʺ��������Ĺ���ǿ���Լ��������������˵õ����������ɫ
		diffuseColor = MatDiffuse * LightDiffuse * diffuseFactor;
		//���ݹ��շ���Ͷ��㷨�������㷴�䷽��
		float3 reflection = reflect(-lightVector, input.Norm);
			//���ݷ��䷽���ӵ㷽���Լ����ʵľ���ⷴ��ϵ�������㾵�淴������
			float specularFactor = pow(max(dot(reflection, input.ViewDirection.xyz), 0.0f), MatPower);
		//���ʵľ��淴���ʣ������ǿ���Լ����淴��������˵õ��������ɫ
		specularColor = MatSpecular * LightSpecular * specularFactor;
	}

	float d = input.LightVector.w;   //���վ���
	//����˥������
	float att = LightAtt0 + LightAtt1 * d + LightAtt2 * d * d;
	//������ɫ�ɾ��⣬�����䣬�������ɫ������Ӻ��ٳ��Ծ���˥�����ӵõ�
	//��ȡ������ɫ
	finalColor = saturate(ambientColor + diffuseColor + specularColor) / att;
	float4 texColor = Texture.Sample(Sampler, input.Tex);//��ȡ������ɫ
	finalColor = finalColor  * texColor;//������ɫ�������ɫ��˵õ�������ɫ
	finalColor.a = texColor.a * MatDiffuse.a;//��ȡalphaֵ��͸����
	return finalColor;
}

// �۹��������ɫ��
float4 PSSpotLight(VS_OUTPUT input) : SV_Target
{
	float4 finalColor;        //������ɫ�����������ɫ�����յ���ɫ
	float4 ambientColor, diffuseColor, specularColor;  //���������⣬�����䣬�������ɫ

	//�����������ڶ���Ĺ�������
	float3 lightVector = input.LightVector.xyz;
		lightVector = normalize(lightVector); //��һ��
	float d = input.LightVector.w;        //���վ���
	float3 lightDirection = LightDirection.xyz;  //���շ���
		lightDirection = normalize(lightDirection);  //��һ��

	//�жϹ�������
	float cosTheta = dot(-lightVector, lightDirection);
	//��������λ��Բ׶����������֮�⣬�򲻲�������
	if (cosTheta < cos(LightBeta / 2))
		return float4(0.0f, 0.0f, 0.0f, 1.0f);
	//�ò��ʻ����ⷴ���ʺͻ�����ǿ����˵õ���������ɫ
	ambientColor = MatAmbient * LightAmbient;
	//�����㷨�����͹��շ�����е�˵õ������������
	float diffuseFactor = dot(lightVector, input.Norm);
	if (diffuseFactor > 0.0f)    //�����������>0��ʾ���Ǳ�����
	{
		//�ò��ʵ��������ķ����ʺ��������Ĺ���ǿ���Լ��������������˵õ����������ɫ
		diffuseColor = MatDiffuse * LightDiffuse * diffuseFactor;
		//���ݹ��շ���Ͷ��㷨�������㷴�䷽��
		float3 reflection = reflect(-lightVector, input.Norm);
			//���ݷ��䷽���ӵ㷽���Լ����ʵľ���ⷴ��ϵ�������㾵�淴������
			float specularFactor = pow(max(dot(reflection, input.ViewDirection.xyz), 0.0f), MatPower);
		//���ʵľ��淴���ʣ������ǿ���Լ����淴��������˵õ��������ɫ
		specularColor = MatSpecular * LightSpecular * specularFactor;
	}

	//����˥������
	float att = LightAtt0 + LightAtt1 * d + LightAtt2 * d * d;
	if (cosTheta > cos(LightAlpha / 2)) //�������λ����׶����
	{
		finalColor = saturate(ambientColor + diffuseColor + specularColor) / att;
	}
	else if (cosTheta >= cos(LightBeta / 2) && cosTheta <= cos(LightAlpha / 2))//�������λ����׶�����׶��֮��
	{
		//��׶��˥������
		float spotFactor = pow((cosTheta - cos(LightBeta / 2)) / (cos(LightAlpha / 2) - cos(LightBeta / 2)), 1);
		finalColor = spotFactor * saturate(ambientColor + diffuseColor + specularColor) / att;
	}
	float4 texColor = Texture.Sample(Sampler, input.Tex);//��ȡ������ɫ
	finalColor = finalColor  * texColor;//������ɫ�������ɫ��˵õ�������ɫ
	finalColor.a = texColor.a * MatDiffuse.a;//��ȡalphaֵ��͸����
	return finalColor;
}

//////////////////////////////////////////////////////////////////////////
// �������Technique
// ��ЩTechnique����������������ɫ����ͬ
//////////////////////////////////////////////////////////////////////////
technique11 TexTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

//�����Technique
technique11 T_DirLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PSDirectionalLight()));
	}
}

//���ԴTechnique
technique11 T_PointLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PSPointLight()));
	}
}

//�۹�ƹ�ԴTechnique
technique11 T_SpotLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PSSpotLight()));
	}
}
