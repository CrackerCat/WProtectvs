// TestAsmjit.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "AsmJit.h"
using namespace asmjit;

typedef int(*FUNC)();

void MinimalExample()
{
	JitRuntime rt;
	CodeHolder code;  //�洢������ض�λ��Ϣ
	code.init(rt.getCodeInfo());//

	X86Assembler assm(&code); //���������������code
	assm.mov(x86::eax, 1); //mov eax, 1
	assm.ret(); //�Ӻ�������
	// **** X86Assember ������Ҫ�����Ա����ٺ��ͷ� 

	FUNC pfn = 0;
	Error err = rt.add(&pfn, &code); //�������Ĵ�����ӵ�runtime
	if (err)
	{
		return; //����
	}
	// *** CodeHoder ������Ҫ�����Ա�����

	int result = pfn(); //ִ�д���
	printf("%d\r\n", result);

	rt.release(pfn);
}

/*
* ������
*/
void UsingOpransExample(X86Assembler& assm)
{
	// ����������
	X86Gp dst = x86::ecx;
	X86Gp src = x86::rax;
	X86Gp idx = x86::gpq(10); //��ȡr10
	X86Mem m = x86::ptr(src, idx); //�����ڴ��ַ[src+idx] - ����[rax+r10]

	//���� m
	m.getIndexType(); //����X86Reg::kRegGpq
	m.getIndexId(); //���� 10 ��r10��

	//���¹���mem�е�idx
	X86Gp idx_2 = X86Gp::fromTypeAndId(m.getIndexType(), m.getIndexId());
	idx == idx_2; //true, ��ȫ��ͬ

	Operand op = m;
	op.isMem(); //True,����ǿת��mem��x86mem

	m == op; //true, op ��mm�ݿ���
	static_cast<Mem&>(op).addOffset(1); //��ȫ��Ч��
	m == op; //False, opָ��[rax+r10+1], ������[rax+r10]

	//����mov
	assm.mov(dst, m);//���Ͱ�ȫ
	//assm.mov(dst, op); //û���������

	assm.emit(X86Inst::kIdMov, dst, m); //����ʹ�ã��������Ͳ���ȫ
	assm.emit(X86Inst::kIdMov, dst, op);//Ҳ���ԣ�emit�����Ͳ���ȫ�ģ����ҿ��Զ�̬ʹ��
}

typedef int(*PFN_SUM)(int* arr, int nCount);

void AssemblerExample()
{

	JitRuntime rt;
	CodeHolder code;  //�洢������ض�λ��Ϣ
	code.init(rt.getCodeInfo());//

	X86Assembler a(&code); //���������������code

	X86Gp arr, cnt;
	X86Gp sum = x86::eax; //eax�����ֵ

	arr = x86::edx; //��������ָ��
	cnt = x86::ecx; //���������

	a.push(x86::ecx);
	a.push(x86::edx);

	a.mov(arr, x86::ptr(x86::esp, 12)); //ȡ��һ������
	a.mov(cnt, x86::ptr(x86::esp, 16)); //ȥ�ڶ�������

	Label lblLoop = a.newLabel(); //Ϊ�˹���ѭ����������Ҫ��ʶ��
	Label lblExit = a.newLabel();

	a.xor_(sum, sum); //����sum�Ĵ���
	a.test(cnt, cnt); //�߽���
	a.jz(lblExit); // if cnt == 0 jmp Exit
	
	a.bind(lblLoop); //ѭ����������ʼ
	a.add(sum, x86::dword_ptr(arr)); //sum += [arr]
	a.add(arr, 4); //arr++
	a.dec(cnt); //cnt --
	a.jnz(lblLoop);// if cnt != 0 jmp Loop

	a.bind(lblExit); //�˳��߽�
	a.pop(x86::edx);
	a.pop(x86::ecx);
	a.ret();//���� sum(eax)


	PFN_SUM pfnSum = 0;
	Error err = rt.add(&pfnSum, &code); //�������Ĵ�����ӵ�runtime
	if (err)
	{
		return; //����
	}
	// *** CodeHoder ������Ҫ�����Ա�����
	int anArr[10] = {1,2,3,4,5,6,7,8,9,10};
	int result = pfnSum(anArr, 10); //ִ�д���
	printf("%d\r\n", result);
}

typedef void(*PFN_SUMINTS)(int* dst, const int* a, const int* b);
void RelocationExample()
{
	CodeHolder code;
	code.init(CodeInfo(ArchInfo::kTypeHost));//ʹ�ñ��ش�����
	X86Assembler a(&code);

	X86Gp dst = x86::eax;
	X86Gp src_a = x86::ecx;
	X86Gp src_b = x86::edx;
	a.mov(dst, x86::dword_ptr(x86::esp, 4)); //��ȡĿ��ָ��
	a.mov(src_a, x86::dword_ptr(x86::esp, 8)); //��ȡԴָ��
	a.mov(src_b, x86::dword_ptr(x86::esp, 12)); //��ȡԴָ��

	a.movdqu(x86::xmm0, x86::ptr(src_a)); //��[src_a]����4��int��xmm0
	a.movdqu(x86::xmm1, x86::ptr(src_b));
	a.paddd(x86::xmm0, x86::xmm1); //���
	a.movdqu(x86::ptr(dst), x86::xmm0);//�洢��[dst]
	a.ret();

	size_t size = code.getCodeSize(); //��ȡ����Ĵ�С�����

	VMemMgr vm;
	void* p = vm.alloc(size);
	size_t ssre = code.relocate(p); //�ض�λ���洢��p
	int  intA[4] = { 1, 2, 3, 4 };
	int intB[4] = { 5, 6 , 7, 8 };
	int intC[4];
	((PFN_SUMINTS)p)(intC, intA, intB);

}

typedef int(*PFN_TEST)();
void CompilerBasic()
{
	JitRuntime rt;

	CodeHolder code;
	code.init(CodeInfo(ArchInfo::kTypeHost));//ʹ�ñ��ش�����
	X86Compiler cc(&code);

	cc.addFunc(FuncSignature0<int>()); //����ԭ�ͣ� int fn(void);

	X86Gp vReg = cc.newGpd(); //����һ��32�ļĴ���
	cc.mov(vReg, 1); 
	cc.ret(vReg);

	cc.endFunc();//��������
	cc.finalize(); //��cc�����ݽ��л��

	PFN_TEST pfnSum = 0;
	Error err = rt.add(&pfnSum, &code); //�������Ĵ�����ӵ�runtime
	if (err)
	{
		return; //����
	}
	// *** CodeHoder ������Ҫ�����Ա�����
	int anArr[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	int result = pfnSum(); //ִ�д���
	printf("%d\r\n", result);
}


typedef int(*PFN_MEMCPY)(uint32_t* dst, const uint32_t* src, size_t count);
void CompilerMemcpy()
{
	JitRuntime rt;

	CodeHolder code;
	code.init(CodeInfo(ArchInfo::kTypeHost));//ʹ�ñ��ش�����
	X86Compiler cc(&code);

	cc.addFunc(FuncSignature3<void, uint32_t*, uint32_t, size_t>());

	Label lblLoop = cc.newLabel();
	Label lblExit = cc.newLabel();

	X86Gp dst = cc.newIntPtr();
	X86Gp src = cc.newIntPtr();
	X86Gp cnt = cc.newIntPtr();


	cc.setArg(0, src);
	cc.setArg(1, dst);
	cc.setArg(2, cnt);

	cc.test(cnt, cnt);
	cc.jz(lblExit);
	cc.bind(lblLoop);

	X86Gp tmp = cc.newInt32();
	cc.mov(tmp, x86::dword_ptr(src));
	cc.mov(x86::dword_ptr(dst), tmp);

	cc.add(src, 4);
	cc.add(dst, 4);

	cc.dec(cnt);
	cc.jnz(lblLoop);

	cc.bind(lblExit);
	cc.endFunc();

	cc.finalize();

	PFN_MEMCPY pfnSum = 0;
	Error err = rt.add(&pfnSum, &code); //�������Ĵ�����ӵ�runtime
	if (err)
	{
		return; //����
	}
	// *** CodeHoder ������Ҫ�����Ա�����
	uint32_t anArr[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	uint32_t anArrd[10] = { 0 };
	size_t result = pfnSum(anArr, anArrd, 10); //ִ�д���
	printf("%d\r\n", result);
}

int _tmain(int argc, _TCHAR* argv[])
{
	//MinimalExample();
//	AssemblerExample();
	//RelocationExample();
	CompilerMemcpy();

	JitRuntime rt;
	CodeHolder code;  //�洢������ض�λ��Ϣ
	code.init(rt.getCodeInfo());//

	X86Assembler assm(&code); //���������������code
	UsingOpransExample(assm);

	return 0;
}

