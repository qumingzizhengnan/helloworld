#ifndef LIGHT_H_H
#define LIGHT_H_H

#include <xnamath.h>

//���ǲ��ʶԸ��ֹ�ķ�����
struct Material{
	XMFLOAT4 ambient; //���ʻ����ⷴ����
	XMFLOAT4 diffuse; //����������ⷴ����
	XMFLOAT4 specular;//���ʾ���ⷴ����
	float    power;   //����ⷴ��ϵ��
};

//��Դ�����������������3�ֹ�Դ����������
//������ÿ�����Զ����õ������緽���Ͳ����õ���Դλ���Լ�˥�����ӵ�
struct Light{
	int type;//��Դ���ͣ�����⣺0�����Դ��1���۹�ƣ�2

	XMFLOAT4 position;//��Դλ��
	XMFLOAT4 direction;//����λ��
	XMFLOAT4 ambient;//������ǿ��
	XMFLOAT4 diffuse;//�������ǿ��
	XMFLOAT4 specular;//�����ǿ��

	float attenuation0;//����˥������
	float attenuation1;//һ��˥������
	float attenuation2;//����˥������
	float alpha;      //�۹����׶�Ƕ�
	float beta;       //�۹����׶�Ƕ�
	float fallOff;    //�۹��˥��ϡ��

};

#endif