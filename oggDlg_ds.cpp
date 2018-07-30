#include "stdafx.h"
#include "ogg.h"
#include "oggDlg.h"
//#include <math.h>
//#include <vorbis/codec.h>
//#include <vorbis/vorbisfile.h>
//#include <MMSystem.h>
#include "dsound.h"
//#include "afxmt.h"
//#include "Douga.h"
//#include "itiran.h"
//#include "itiran_FC.h"
//#include "itiran_YSF.h"
//#include "itiran_YS6.h"
//#include "itiran_YSO.h"
//#include "vfw.h"
//#include <direct.h>
//#include "Folder.h"
//#include "dsound.h"

#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <Audioclient.h>
#include <endpointvolume.h>
#include <FunctionDiscoveryKeys_devpkey.h>

extern int fade1;
extern 	LPDIRECTSOUND8 m_ds;
extern 	LPDIRECTSOUNDBUFFER m_dsb1;
extern 	LPDIRECTSOUNDBUFFER8 m_dsb;
extern 	LPDIRECTSOUND3DBUFFER m_dsb3d;
extern	LPDIRECTSOUNDBUFFER m_p;
extern LPDIRECTSOUND3DBUFFER m_lpDS3DBuffer;

extern int	playf;
extern void ReleaseOggVorbis(char **);
extern char *ogg;
extern DWORD hw;
extern HANDLE hNotifyEvent[2];
extern LPDIRECTSOUNDNOTIFY dsnf1;
extern LPDIRECTSOUNDNOTIFY dsnf2;
extern UINT HandleNotifications(LPVOID lpvoid);
extern UINT WASAPIHandleNotifications(LPVOID lpvoid);
extern ULONG WAVDALen;
extern UINT ttt;
extern int wavch,wavbit, wavsam;
#define BUFSZ			(2048*6)
#define HIGHDIV			4
#define BUFSZH			(BUFSZ/HIGHDIV)
#define SQRT_BUFSZ2		64
#define M_PI			3.1415926535897932384
#define ABS(N)			( (N)<0 ? -(N) : (N) )
#define OUTPUT_BUFFER_SIZE  BUFSZ
#define OUTPUT_BUFFER_NUM   10
extern void playwavds(BYTE*bw);
extern int playwavds2(BYTE*bw,int len);
extern BOOL playwavadpcm(BYTE* bw,int old,int l1,int l2);
extern int mode;
extern save savedata;
LPDIRECTSOUND3DLISTENER m_listener=NULL;



#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

IMMDeviceEnumerator *deviceEnumerator = NULL;
IMMDeviceCollection *pDeviceCollection = NULL;
IMMDevice *pDevice = NULL;
IAudioClient *pAudioClient = NULL;
IAudioRenderClient *pRenderClient = NULL;
REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
WAVEFORMATEX *pwfx = NULL;
UINT32 bufferFrameCount;


CString COggDlg::init(HWND hwnd,int sm)
{
	DirectSoundCreate8(NULL,&m_ds,NULL);
	if(m_ds==NULL) return _T("DirectSoundを生成できません。\nDirectX7が正常にインストールされているか確認してください。");
	if(m_ds->SetCooperativeLevel(hwnd,DSSCL_PRIORITY)!=DS_OK){
		return _T("DirectSoundの強調レベルを設定できません。\nDirectX7が正常にインストールされているか確認してください。");
	}
	hw=0;
//	ZeroMemory(&d,sizeof(d));d.dwSize=sizeof(d);HRESULT r =m_ds->GetCaps(&d);
//	if(r!=DS_OK){
//		return "DirectSoundの情報を獲得出来ません。\nDirectX7が正常にインストールされているか確認してください。";
//	}
//	if(d.dwFlags & (DSCAPS_SECONDARYSTEREO|DSCAPS_PRIMARYSTEREO |DSCAPS_PRIMARY16BIT) && d.dwFreeHwMemBytes!=0){
//		hw=DSBCAPS_LOCHARDWARE;
//	}::timeSetEvent
	m_p=NULL;
	DSBUFFERDESC dss;
	ZeroMemory(&dss,sizeof(dss));
	dss.dwSize=sizeof(dss);
//	dss.dwFlags=DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY|DSBCAPS_PRIMARYBUFFER|hw;
	dss.dwFlags=DSBCAPS_PRIMARYBUFFER;
	dss.lpwfxFormat=NULL;
	dss.dwBufferBytes=0;
	if(m_ds->CreateSoundBuffer(&dss,&m_p,NULL)!=DS_OK){
		return _T("DirectSoundのプライマリバッファを生成できません。\nDirectX7が正常にインストールされているか確認してください。");
	}
	if(m_p!=NULL){
//		//PCMWAVEFORMAT p;
		WAVEFORMATEX p;
		ZeroMemory(&p,sizeof(p));
		p.wFormatTag=WAVE_FORMAT_PCM;
		p.nChannels= wavch;
		p.nSamplesPerSec= wavbit;
		p.wBitsPerSample = wavsam;
		p.nBlockAlign = p.nChannels * p.wBitsPerSample / 8;
		p.nAvgBytesPerSec = p.nSamplesPerSec * p.nBlockAlign;
		p.cbSize = 0;
		m_p->SetFormat(&p);
	}
	//m_p->QueryInterface(IID_IDirectSound3DListener, (LPVOID*)&m_listener);
	//m_listener->SetPosition(0.0f, 0.0f, 0.0f, DS3D_IMMEDIATE);

	return _T("");
}


extern void DoEvent();
/*
void DoEvent()
{
	MSG msg;
	for(;;){
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}else return;
	}
}
*/
void COggDlg::Vol(int vol)
{
//	if(pAudioClient==NULL)
//		m_dsb->SetVolume(vol);
//	else
//		pAudioClient->
}

void COggDlg::Closeds()
{
//	fade1=1;
	if(m_dsb){
		m_dsb->Stop();
		if(m_dsb3d != NULL){m_dsb3d->Release();m_dsb3d =NULL;}
		if(m_dsb != NULL){m_dsb->Release();m_dsb =NULL;}
	}
	if (pAudioClient) {
		pAudioClient->Stop();
		pRenderClient->Release(); pRenderClient = NULL;
		pAudioClient->Release(); pAudioClient = NULL;
	}
}

BOOL COggDlg::ReleaseDXSound(void)
{
	if(m_ds){
		Closeds();
		if(m_dsb3d != NULL){m_dsb3d->Release();m_dsb3d =NULL;}
		if(m_dsb != NULL) {m_dsb->Release();m_dsb=NULL;}
		if (m_dsb1 != NULL) { m_dsb1->Release(); m_dsb1 = NULL; }
		if(m_lpDS3DBuffer != NULL){m_lpDS3DBuffer->Release();}
		m_dsb =NULL;
		m_lpDS3DBuffer= NULL;
		if(m_p!=NULL){m_p->Release();m_p=NULL;}

		if(m_ds){
			m_ds->Release();
			m_ds = NULL;
		}
	}
	if (pAudioClient) {
		pAudioClient->Stop();
		if (pRenderClient) { pRenderClient->Release(); pRenderClient = NULL; }
		pAudioClient->Release(); pAudioClient = NULL;
		pDevice->Release(); pDevice = NULL;

	}
	return TRUE;
}

extern int playwavds2(BYTE* bw,int old,int l1,int l2);
extern int playwavkpi(BYTE* bw,int old,int l1,int l2);
extern int playwavmp3(BYTE* bw,int old,int l1,int l2);
extern int playwavflac(BYTE* bw, int old, int l1, int l2);
extern int playwavm4a(BYTE* bw, int old, int l1, int l2);
extern BYTE bufwav3[OUTPUT_BUFFER_SIZE*OUTPUT_BUFFER_NUM*6];
extern int ps;
extern COggDlg *og;
extern BOOL thn;
extern BOOL thn1;
extern int endf;
extern int lenl;
extern int fade1;
extern BOOL sek;
extern int wavch,wavbit;
//スレッド
int syukai=0,syukai2=0;
UINT HandleNotifications(LPVOID)
{
	DWORD hr = DS_OK;
	DWORD hRet = 0;
	thn=FALSE;
	thn1=FALSE;
	char* pdsb1;
	char* pdsb2;
	syukai=0;
	HANDLE ev[] = {(HANDLE)og->timer};
	ULONG PlayCursor,WriteCursor=0,oldw=OUTPUT_BUFFER_SIZE*2,oldw2;
	m_dsb->SetCurrentPosition(0);
	if(mode==-10){
		oldw=OUTPUT_BUFFER_SIZE*2;
		og->timer.SetEvent();
	}
	fade1=0;
	for(;;){
		DWORD  dwDataLen = WAVDALen/10;
		if(syukai==2) {thn=TRUE;AfxEndThread(0);}
		if(syukai==1) {syukai2=1;continue;}
			::WaitForMultipleObjects(1, ev, FALSE, 40);
			if (sek == 1) {
				if (m_dsb)m_dsb->Stop();
				oldw = 0;
				if ((mode >= 10 && mode <= 20) || mode < -10) {
					playwavadpcm(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
				}
				else if (mode == -10) {
					playwavmp3(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
				}
				else if (mode == -3) {
					playwavkpi(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
				}
				else if (mode == -8) {
					playwavflac(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
				}
				else if (mode == -9) {
					playwavm4a(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
				}
				else {
					playwavds2(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
				}
				oldw = OUTPUT_BUFFER_SIZE / 12 * wavch * 2; WriteCursor = OUTPUT_BUFFER_SIZE / 12 * wavch * 2;
				if(m_dsb)m_dsb->SetCurrentPosition(0);
				if(m_dsb)m_dsb->Play(0,0,DSBPLAY_LOOPING);
				sek=FALSE;
				//break;
			}
			if(thn1) {thn=TRUE;AfxEndThread(0);}
//		}
		if(ps==1) continue;
		if(m_dsb)m_dsb->GetCurrentPosition(&PlayCursor, &WriteCursor);//再生位置取得
		int len1=0,len2=0,len3,len4,len5;
		len1=(int)WriteCursor-(int)oldw;//書き込み範囲取得10
		len2=0;
		if (len1 == 0 && len2 == 0) continue;
		if(len1<0){
			len1=OUTPUT_BUFFER_SIZE*OUTPUT_BUFFER_NUM-oldw; len2= WriteCursor;}
		if(len2<0)
			len2=0;
		len4=len1+len2;
		if((mode>=10 && mode<=20) || mode<-10)
			playwavadpcm(bufwav3,oldw,len1,len2);//データ獲得
		else if(mode==-10)
			len4=playwavmp3(bufwav3,oldw,len1,len2);//データ獲得
		else if(mode==-3)
			len4=playwavkpi(bufwav3,oldw,len1,len2);//データ獲得
		else if (mode == -8)
			playwavflac(bufwav3, oldw, len1, len2);//データ獲得
		else if (mode == -9)
			playwavm4a(bufwav3, oldw, len1, len2);//データ獲得
		else
			playwavds2(bufwav3,oldw,len1,len2);//データ獲得
		len5 = len4;
		oldw2 = oldw;
		if (fade1) {
			for (int jj = 0; jj < PlayCursor/wavch; jj++) {
				if (wavch == 1) {
					bufwav3[jj] = 0x80;
				}
				if (wavch == 2) {
					bufwav3[jj] = 0x00;
					bufwav3[jj+1] = 0x80;
				}
				if (wavch == 3) {
					bufwav3[jj] = 0x00;
					bufwav3[jj+1] = 0x00;
					bufwav3[jj+2] = 0x80;
				}
			}
		}
		if(m_dsb){
			m_dsb->Lock(oldw,len4,(LPVOID *)&pdsb1,(DWORD*)&len3,(LPVOID *)&pdsb2,(DWORD*)&len4,0);
			thn=FALSE;
			Sleep(20);
			thn=FALSE;
			memcpy(pdsb1,bufwav3+oldw,len3);
			if(len4!=0)memcpy(pdsb2,bufwav3,len4);
			len5 += len3 + len4;
			if(m_dsb)m_dsb->Unlock(pdsb1,len3,pdsb2,len4);
			oldw=WriteCursor;
		}
		if(fade1){
			playf = 1;
			thn = FALSE;
			int wavv = wavbit;
//			if (wavbit < 44100) wavv = 44100;
			Sleep(777 * (44100 / wavv));

			m_dsb->SetVolume(DSBVOLUME_MIN);
			m_dsb->Stop();
			og->OnPause();
			playf=0;
			thn=TRUE;

			AfxEndThread(0);
			return 0;
		}
	}

} //handlenotifications()

  
  
  
  
  
  
  
  
  
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioClock = __uuidof(IAudioClock);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

UINT32 bufsize;

int COggDlg::WASAPIInit()
{
	return 0;
	CoInitialize(NULL);
	::CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&deviceEnumerator);
	if (deviceEnumerator == NULL) {
		return 0;
	}
	deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	pDevice->Activate(IID_IAudioClient, CLSCTX_ALL,	NULL, (void**)&pAudioClient);
	return 1;
}

template< typename T, class TFreePolicy >
class base_memory
{
private:
	T *FMemory;
public:
	base_memory(T* AMemory = NULL)
		: FMemory(AMemory) {}
	virtual ~base_memory(void)
	{
		reset();
	}
	T* release(void)
	{
		T *tmp = FMemory;
		FMemory = NULL;
		return tmp;
	}
	void reset(T* AMemory = NULL)
	{
		if (AMemory != FMemory)
		{
			if (NULL != FMemory)
				TFreePolicy(FMemory);
			FMemory = AMemory;
		}
	}
	operator T* ()
	{
		return FMemory;
	}
	T* get() { return FMemory; }
	T* operator ->() { return FMemory; }
	T** operator&(void)
	{
		return &FMemory;
	}
};

struct co_task_memory_free_policy
{
	template< typename T >
	void operator()(const T* AMemory) const
	{
		if (NULL != AMemory)
			::CoTaskMemFree(AMemory);
	}
};

template< typename T >
class co_task_memory : public base_memory< T,
	co_task_memory_free_policy >
{
public:
	co_task_memory(T* AMemory = NULL)
		: base_memory< T, co_task_memory_free_policy >(AMemory)
	{ }
};

void COggDlg::WASAPIChange(WAVEFORMATEX* pwf)
{
	if (pAudioClient) pAudioClient->Stop();
	co_task_memory<WAVEFORMATEX>  alt_format;
	REFERENCE_TIME buffer_period = 40 /* ms */ * 10000;
	REFERENCE_TIME buffer_duration = buffer_period * 4;
	int ret = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*)pwf, &alt_format);
	if (FAILED(ret)) {
		MessageBox(L"未サポートのフォーマット¥n");
		return;
	}
	ret=pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST, buffer_duration, buffer_period, pwf, NULL);
	ret=pAudioClient->GetBufferSize(&bufsize);
	ret=pAudioClient->GetService(IID_PPV_ARGS(&pRenderClient));
}

UINT WASAPIHandleNotifications(LPVOID lpvoid)
{
	DWORD hr = DS_OK;
	DWORD hRet = 0;
	thn = FALSE;
	thn1 = FALSE;
	BYTE* pdsb;
	syukai = 0;
	HANDLE ev[] = { (HANDLE)og->timer };
	ULONG PlayCursor, WriteCursor = 0, oldw = OUTPUT_BUFFER_SIZE * 2, oldw2;
	if (mode == -10) {
		oldw = OUTPUT_BUFFER_SIZE * 2;
		og->timer.SetEvent();
	}
	fade1 = 0;
	for (;;) {
		DWORD  dwDataLen = WAVDALen / 10;
		if (syukai == 2) { thn = TRUE; AfxEndThread(0); }
		if (syukai == 1) { syukai2 = 1; continue; }
		::WaitForMultipleObjects(1, ev, FALSE, 40);
		if (sek == 1) {
			if (pAudioClient) pAudioClient->Stop();
			oldw = 0;
			if ((mode >= 10 && mode <= 20) || mode < -10) {
				playwavadpcm(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
			}
			else if (mode == -10) {
				playwavmp3(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
			}
			else if (mode == -3) {
				playwavkpi(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
			}
			else if (mode == -8) {
				playwavflac(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
			}
			else if (mode == -9) {
				playwavm4a(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
			}
			else {
				playwavds2(bufwav3, oldw, OUTPUT_BUFFER_SIZE / 12 * wavch * 2, 0);//データ獲得
			}
			oldw = OUTPUT_BUFFER_SIZE / 12 * wavch * 2; WriteCursor = OUTPUT_BUFFER_SIZE / 12 * wavch * 2;
			if (pAudioClient)pAudioClient->Start();
			//break;
		}
		if (thn1) { thn = TRUE; AfxEndThread(0); }
		//		}
		if (ps == 1) continue;
		pAudioClient->GetBufferSize(&bufferFrameCount);
		int len1 = 0, len2 = 0, len3=0, len4, len5;
		len1 = bufferFrameCount;
		oldw += len1;
		if (oldw > (OUTPUT_BUFFER_SIZE * OUTPUT_BUFFER_NUM)) { oldw -= OUTPUT_BUFFER_SIZE*OUTPUT_BUFFER_NUM; }
		len2 = 0;
		len4 = len1 + len2;
		if ((mode >= 10 && mode <= 20) || mode < -10)
			playwavadpcm(bufwav3, oldw, len1, len2);//データ獲得
		else if (mode == -10)
			len4 = playwavmp3(bufwav3, oldw, len1, len2);//データ獲得
		else if (mode == -3)
			len4 = playwavkpi(bufwav3, oldw, len1, len2);//データ獲得
		else if (mode == -8)
			len4 = playwavflac(bufwav3, oldw, len1, len2);//データ獲得
		else if (mode == -9)
			len4 = playwavm4a(bufwav3, oldw, len1, len2);//データ獲得
		else
			len4 = playwavds2(bufwav3, oldw, len1, len2);//データ獲得
		pRenderClient->GetBuffer(bufferFrameCount, &pdsb);
		memcpy(bufwav3 + len3, pdsb, sizeof(BYTE));
		len3 = oldw;
		if (len4 == 0) {
			pRenderClient->ReleaseBuffer(bufferFrameCount, AUDCLNT_BUFFERFLAGS_SILENT);
			break;
		}
		else {
			pRenderClient->ReleaseBuffer(bufferFrameCount, 0);
		}
		if (fade1) {
			playf = 1;
			thn = FALSE;
			Sleep(1200 * 44100 / wavbit);
			og->OnPause();
			playf = 0;
			thn = TRUE;

			AfxEndThread(0);
			return 0;
		}
	}
	return 0;
}