// DownloadJet.h: interface for the CDownloadJet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOWNLOADJET_H__DB5BA134_790C_4602_878C_D91939CEE761__INCLUDED_)
#define AFX_DOWNLOADJET_H__DB5BA134_790C_4602_878C_D91939CEE761__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "TE_Socket.h"
#include "ListItem.h"

//�Զ��������߳�ͨ����Ϣ
#define WM_USER_DOWNLOAD_THREAD_MSG_BASE		WM_USER + 500

//�������̷߳�����״̬��Ϣ��
#define WM_USER_DOWNLOAD_THREAD_GENERAL_INFO	WM_USER_DOWNLOAD_THREAD_MSG_BASE + 0	

//�����߳̿�ʼ�������ݡ�
#define WM_USER_DOWNLOAD_THREAD_START_RECV		WM_USER_DOWNLOAD_THREAD_MSG_BASE + 1	

//�����߳���ɵ�ǰָ����Χ������
#define WM_USER_DOWNLOAD_THREAD_COMPLETED		WM_USER_DOWNLOAD_THREAD_MSG_BASE + 2	

//�����̳߳��ֲ��ɻָ��Ĵ���
#define WM_USER_DOWNLOAD_THREAD_ERROR			WM_USER_DOWNLOAD_THREAD_MSG_BASE + 3	

//�����߳�ʧ�ܣ����Կ������ԡ�
#define WM_USER_DOWNLOAD_THREAD_FAILED			WM_USER_DOWNLOAD_THREAD_MSG_BASE + 4	

//�����߳̽������ݳɹ�
#define WM_USER_DOWNLOAD_THREAD_RECV_OK			WM_USER_DOWNLOAD_THREAD_MSG_BASE + 5	

//�����̱߳��û�ȡ��
#define WM_USER_DOWNLOAD_THREAD_CANCELED		WM_USER_DOWNLOAD_THREAD_MSG_BASE + 6	


//ͨ����Ϣ���
#define STATE_MSG								0
#define COMMAND_MSG								1
#define ERROR_MSG								2
#define SERVER_MSG								3

// �ṹ����
typedef struct _tagDownloadStatus
{
	UINT	nStatusType;
	DWORD	dwFileSize;
	DWORD	dwFileDownloadedSize;
	CString	strFileName;
//	UINT	nErrorCount;	
//	CString strError;
//	DWORD	dwErrorCode;
}DOWNLOADSTATUS,*PDOWNLOADSTATUS;

// ��������
// ȱʡ��ʱ����
const DWORD DOWNLOAD_CONNECT_TIMEOUT= 10*1000;// 10��
const DWORD DOWNLOAD_SEND_TIMEOUT	= 10*1000;// 10��
const DWORD DOWNLOAD_RECV_TIMEOUT	= 10*1000;// 10��

// ���ؽ��
const UINT	DOWNLOAD_RESULT_SUCCESS	= 0;	// �ɹ�
const UINT	DOWNLOAD_RESULT_SAMEAS	= 1;	// Ҫ���ص��ļ��Ѿ����ڲ�����Զ���ļ�һ�£���������
const UINT	DOWNLOAD_RESULT_STOP	= 2;	// ��;ֹͣ(�û��ж�)
const UINT	DOWNLOAD_RESULT_FAIL	= 3;	// ����ʧ��

// ��������
const UINT SENDREQUEST_SUCCESS	= 0; // �ɹ�
const UINT SENDREQUEST_ERROR	= 1; // һ��������󣬿�������
const UINT SENDREQUEST_STOP		= 2; // ��;ֹͣ(�û��ж�) (��������)
const UINT SENDREQUEST_FAIL		= 3; // ʧ�� (��������)	 

// ��Ϣ
const WPARAM	MSG_INTERNET_STATUS	= (WPARAM)1;
const WPARAM	MSG_DOWNLOAD_STATUS = (WPARAM)2;
const WPARAM	MSG_DOWNLOAD_RESULT = (WPARAM)3;
const WPARAM	MSG_DOWNLOAD_MAX	= (WPARAM)32; //�������ɹ�����

//����״̬
const UINT		STATUS_TYPE_FILENAME			= 1;
const UINT		STATUS_TYPE_FILESIZE			= 2;
const UINT		STATUS_TYPE_FILEDOWNLOADEDSIZE	= 3;
const UINT		STATUS_TYPE_ERROR_COUNT			= 4;
const UINT		STATUS_TYPE_ERROR_CODE			= 5;
const UINT		STATUS_TYPE_ERROR_STRING		= 6;

// �������
const UINT RETRY_TYPE_NONE	= 0;
const UINT RETRY_TYPE_TIMES	= 1;
const UINT RETRY_TYPE_ALWAYS= 2;
//ȱʡ�����Դ���
const UINT DEFAULT_RETRY_MAX= 10; 


// HTTP STATUS CODE����
const UINT	HTTP_OK			= 0;
const UINT	HTTP_ERROR		= 1;
const UINT	HTTP_REDIRECT	= 2;
const UINT	HTTP_FAIL		= 3;

// PROXY������
const UINT	PROXY_NONE			= 0;
const UINT	PROXY_HTTPGET		= 1;
const UINT	PROXY_HTTPCONNECT	= 2;
const UINT	PROXY_SOCKS4		= 3;
const UINT	PROXY_SOCKS4A		= 4;
const UINT	PROXY_SOCKS5		= 5;




class CDownloadItemManager;
class CDownloadJet  
{
public:
	CDownloadJet();
	virtual ~CDownloadJet();

	//�����С 10K
	enum { download_buffer_size = (64 * 1024) };

public:
	virtual BOOL OwnSocket(SOCKET socket) {return (socket == m_hSocket);};
	virtual void Timeout() = 0;
	int GetSocket();
	virtual int DownloadHandler(WPARAM wParam, LPARAM lParam) =0;
	void SetForceDownload(BOOL bForceDownload);
	virtual void SetURL(LPCTSTR lpszDownloadUrl);
	virtual void StopDownload() = 0;
	virtual void StartDownload() = 0;

	BOOL IsSupportResume();
	PBREAKPOSITION GetRange();
	DWORD GetCurDownloadedSize();
	LPCTSTR GetDownloadBuf();
	DWORD GetCurPos();
	DWORD GetContentLength();
	void SetRange(PBREAKPOSITION pBrkPos);
	LPCTSTR GetHeaderInfo();
	void SetAuthorization(LPCTSTR lpszUsername,LPCTSTR lpszPassword,BOOL bAuthorization = TRUE );
	void SetReferer(LPCTSTR lpszReferer);
	void SetUserAgent(LPCTSTR lpszUserAgent);
	void SetTimeout(DWORD dwSendTimeout,DWORD dwReceiveTimeout,DWORD dwConnectTimeout);
	void SetRetry(UINT nRetryType, UINT nRetryDelay=0,UINT nRetryMax = 0);
	virtual void SetProxy(LPCTSTR lpszProxyServer, USHORT nProxyPort,BOOL bProxy=TRUE,BOOL bProxyAuthorization = FALSE,LPCTSTR lpszProxyUsername = NULL,LPCTSTR lpszProxyPassword = NULL,UINT nProxyType = PROXY_HTTPGET) = 0;

protected:
	// ���ز���
	// ������URL
	CString		m_strDownloadUrl;

	// ֹͣ����
	BOOL		m_bStopDownload;

	// ǿ����������(�������е��ļ��Ƿ���Զ���ļ���ͬ)
	BOOL		m_bForceDownload;

	// �Ƿ�֧�ֶϵ�����
	BOOL		m_bSupportResume;

	// ���ش�С
	int			m_iDownloadedBufSize;			// �����Ѿ����صĴ�С

	PBREAKPOSITION m_pBrkPos;
	char		m_szReadBuf[CDownloadJet::download_buffer_size+1];

	DWORD		m_dwHeaderSize;				// ����ͷ�Ĵ�С
	CString		m_strHeader;				// ����ͷ����Ϣ

	// �ļ�����(Զ���ļ�����Ϣ)
	CTime		m_TimeLastModified;

	// Referer
	CString		m_strReferer;
	
	// UserAgent
	CString		m_strUserAgent;

	// ��ʱTIMEOUT	���ӳ�ʱ�����ͳ�ʱ�����ճ�ʱ(��λ������)
	DWORD		m_dwConnectTimeout;	
	DWORD		m_dwReceiveTimeout;
	DWORD		m_dwSendTimeout;

	// ���Ի���
	UINT		m_nRetryType;	//��������(0:������ 1:����һ������ 2:��������)
	UINT		m_nRetryTimes;	//���Դ���
	UINT		m_nRetryDelay;	//�����ӳ�(��λ������)
	UINT		m_nRetryMax;    //���Ե�������

	// ������
	UINT		m_nErrorCount;	//�������
	CString		m_strError;		//������Ϣ

	// �Ƿ������֤ : Request-Header: Authorization
	BOOL		m_bAuthorization;
	CString		m_strUsername;
	CString		m_strPassword;

	// �Ƿ�ʹ�ô��� 
	BOOL		m_bProxy;
	CString		m_strProxyServer;
	USHORT		m_nProxyPort;
	UINT		m_nProxyType;
	
	// �����Ƿ���Ҫ��֤: Request-Header: Proxy-Authorization
	BOOL		m_bProxyAuthorization;
	CString		m_strProxyUsername;
	CString		m_strProxyPassword;


	// ���ع��������õı���
	CString		m_strServer;
	CString		m_strObject;
	USHORT		m_nPort;

	SOCKET		m_hSocket;	// �������ӵ�SOCKET

	CDownloadItemManager *m_pParent;
	DWORD		m_dwContentLength;

protected:	
	void SendCancelMsg();
	void SendResultFailedMsg();
	CString EncapsulateJetInfo(int iType, LPCTSTR lpszContent);
	void SendHeaderInfoMsg(int iType);
	BOOL SendStartDownloadMsg();
	void SendCompletedMsg();
	void SendFailedMsg();
	void SendRecvOKMsg();

};

// ȱʡ�˿ں�
const USHORT DEFAULT_HTTP_PORT = 80;
const USHORT DEFAULT_HTTPS_PORT= 443;
const USHORT DEFAULT_FTP_PORT  = 21;
const USHORT DEFAULT_SOCKS_PORT= 1080;

#endif // !defined(AFX_DOWNLOADJET_H__DB5BA134_790C_4602_878C_D91939CEE761__INCLUDED_)
