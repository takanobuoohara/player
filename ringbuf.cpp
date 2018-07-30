#include "stdafx.h"
#include "ringbuf.h"

KbRingBuffer::KbRingBuffer(DWORD dwInitialSize/*=0*/)
{
    m_hHeap = NULL;
    m_pBuffer = NULL;
    if((int)dwInitialSize <= 0){
        dwInitialSize = 4*1024;
    }
    m_dwReadPos = m_dwWritePos = m_dwWritten = 0;
    m_dwBufferSize = dwInitialSize;
    m_dwAllocSize = 0;

    m_dwLockedSize = 0;
}

KbRingBuffer::~KbRingBuffer(void)
{
    if(m_hHeap){
        HeapDestroy(m_hHeap);
    }
}
void __fastcall KbRingBuffer::Expand(DWORD dwNewSize)
{//�����O�o�b�t�@�̃T�C�Y���g��E�k������
 //�T�C�Y���k������ꍇ�́A�k���O�̃f�[�^�͂��ׂĎ�����
 //�T�C�Y���g�傷��ꍇ�́A�g��O��Ńf�[�^�͎����Ȃ�
    //OutputDebugString("Expand\n");
    BYTE *pNewBuf;
    if(dwNewSize < m_dwBufferSize){//�k������
        m_dwReadPos = m_dwWritePos = m_dwWritten = 0;//�k���O�̃f�[�^�͂��ׂĎ�����
    }
#if 1
    //�r�������͌Ăяo�����ŊǗ�����̂� HEAP_NO_SERIALIZE �Ŗ��Ȃ�
    if(!m_hHeap){
        m_hHeap = ::HeapCreate(HEAP_NO_SERIALIZE, dwNewSize+4096, 0);
        pNewBuf = (BYTE*)HeapAlloc(m_hHeap, HEAP_NO_SERIALIZE, dwNewSize);
    }
    else{
        pNewBuf = (BYTE*)HeapReAlloc(m_hHeap, HEAP_NO_SERIALIZE, m_pBuffer, dwNewSize);
    }
    if(m_dwReadPos > m_dwWritePos || 
       (m_dwReadPos == m_dwWritePos && m_dwWritten != 0)){
        DWORD dwExpandSize = dwNewSize-m_dwBufferSize;
        //memmove(pNewBuf + m_dwReadPos + dwExpandSize, pNewBuf+m_dwReadPos, 
        //            m_dwBufferSize-m_dwReadPos);
        const DWORD *end4 = (DWORD*)(pNewBuf+m_dwReadPos);
        DWORD *src4 = (DWORD*)(pNewBuf+m_dwBufferSize-4);
        DWORD *dst4 = (DWORD*)((BYTE*)src4+dwExpandSize);
        while(src4 >= end4){
            *dst4-- = *src4--;
        }
        BYTE *src = (BYTE*)src4+3;
        BYTE *dst = (BYTE*)dst4;
        while(src >= (const BYTE*)end4){
            *dst-- = *src--;
        }
        m_dwReadPos += dwExpandSize;
    }
#else
    DWORD dwWritten = m_dwWritten;
    if(!m_hHeap){
        m_hHeap = ::HeapCreate(HEAP_NO_SERIALIZE, dwNewSize+4096, 0);
    }
    pNewBuf = (BYTE*)HeapAlloc(m_hHeap, HEAP_NO_SERIALIZE, dwNewSize);
    Read(pNewBuf, dwWritten);
    HeapFree(m_hHeap, 0, m_pBuffer);
    m_dwReadPos = 0;
    m_dwWritePos = m_dwWritten = dwWritten;
#endif
    m_pBuffer = pNewBuf;
    m_dwBufferSize = dwNewSize;
    m_dwAllocSize = dwNewSize;
}

DWORD __fastcall KbRingBuffer::Read(BYTE* buf, DWORD dwSize)
{//�����O�o�b�t�@�̓��e��ǂݎ��
 //���ۂɓǂݎ�����o�C�g����Ԃ�
    if(dwSize > m_dwWritten){
        dwSize = m_dwWritten;
    }
    DWORD dwCopyFirst;//�擪��������̃R�s�[�T�C�Y
    DWORD dwCopyTail; //�I�[��������̃R�s�[�T�C�Y
    dwCopyTail = m_dwAllocSize-m_dwReadPos;
    if(dwCopyTail > dwSize){
        dwCopyTail = dwSize;
    }
    dwCopyFirst = dwSize-dwCopyTail;

    memcpy(buf,m_pBuffer+m_dwReadPos,dwCopyTail);
	memcpy(buf+dwCopyTail,m_pBuffer,dwCopyFirst);
    m_dwReadPos += dwSize;
    if(m_dwReadPos > m_dwAllocSize){
        m_dwReadPos -= m_dwAllocSize;
    }
    m_dwWritten -= dwSize;
    return dwSize;
}
void __fastcall KbRingBuffer::Write(const BYTE* buf, DWORD dwSize)
{//�����O�o�b�t�@�ɏ�������
 //�o�b�t�@�T�C�Y������Ȃ��ꍇ�͊g�傷��
    if(m_dwAllocSize-m_dwWritten < dwSize){
        DWORD dwNewSize = m_dwWritten + dwSize;
        if(dwNewSize < m_dwBufferSize){
            //���ꂪ�Ȃ��� SetSize �ŃT�C�Y�g����w�肵���Ӗ����Ȃ��Ȃ�
            dwNewSize = m_dwBufferSize;
        }
        Expand(dwNewSize);
    }
    DWORD dwCopyFirst;//�擪�����ւ̃R�s�[�T�C�Y
    DWORD dwCopyTail; //�I�[�����ւ̃R�s�[�T�C�Y
    dwCopyTail = m_dwAllocSize-m_dwWritePos;
    if(dwCopyTail > dwSize){
        dwCopyTail = dwSize;
    }
    dwCopyFirst = dwSize-dwCopyTail;
    memcpy(m_pBuffer+m_dwWritePos, buf,dwCopyTail);
	memcpy(m_pBuffer,buf+dwCopyTail,dwCopyFirst);
    m_dwWritePos += dwSize;
    if(m_dwWritePos > m_dwAllocSize){
        m_dwWritePos -= m_dwAllocSize;
    }
    m_dwWritten += dwSize;
}
BYTE* __fastcall KbRingBuffer::LockBuffer(BYTE** ppBuf,DWORD dwSize)
{//�����O�o�b�t�@�ɒ��ڏ�������
 //�߂�l�ւ̏������݂͌Ăяo�����ōs��
 //�o�b�t�@�T�C�Y������Ȃ��ꍇ�͊g�傷��
    if(m_dwAllocSize-m_dwWritten < dwSize){
        DWORD dwNewSize = m_dwWritten + dwSize;
        if(dwNewSize < m_dwBufferSize){
            //���ꂪ�Ȃ��� SetSize �ŃT�C�Y�g����w�肵���Ӗ����Ȃ��Ȃ�
            dwNewSize = m_dwBufferSize;
        }
        Expand(dwNewSize);
    }
    DWORD dwCopyFirst;//�擪�����ւ̃R�s�[�T�C�Y
    DWORD dwCopyTail; //�I�[�����ւ̃R�s�[�T�C�Y
    dwCopyTail = m_dwAllocSize-m_dwWritePos;
    if(dwCopyTail > dwSize){
        dwCopyTail = dwSize;
    }
    dwCopyFirst = dwSize-dwCopyTail;
    if(dwCopyTail == dwSize){
        *ppBuf = m_pBuffer + m_dwWritePos;
    }
    else if(dwCopyFirst == dwSize){
        *ppBuf = m_pBuffer;
    }
    else{//�擪�ƏI�[�̗����ɏ������܂Ȃ���΂Ȃ�Ȃ��ꍇ
        *ppBuf = NULL;
    }
    //m_pLocked = *ppBuf;
    m_dwLockedSize = dwSize;
    return *ppBuf;
}
void __fastcall KbRingBuffer::UnlockBuffer(void)
{//�����O�o�b�t�@�ւ̒��ڏ������ݏI��
    if(m_dwLockedSize){
        m_dwWritePos += m_dwLockedSize;
        if(m_dwWritePos > m_dwAllocSize){
            m_dwWritePos -= m_dwAllocSize;
        }
        m_dwWritten += m_dwLockedSize;
        m_dwLockedSize = 0;
    }
}
void  __fastcall KbRingBuffer::SetSize(DWORD dwSize)
{//�����O�o�b�t�@�̃T�C�Y��ݒ肷��
    if(dwSize < m_dwAllocSize){
        //���݂̃o�b�t�@�T�C�Y�����������T�C�Y���w�肵���ꍇ��
        //�����Ƀo�b�t�@�T�C�Y������������
        //�T�C�Y�ύX�O�̃f�[�^�͂��ׂĎ�����
        Expand(dwSize);
    }
    else{
        //���݂̃o�b�t�@�T�C�Y�����傫������ꍇ�̓T�C�Y�ύX�O
        //�̃f�[�^�͎����Ȃ�
        m_dwBufferSize = dwSize;
        Expand(dwSize);
    }
}