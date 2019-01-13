// DownloadJet.cpp: implementation of the CDownloadJet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "myget.h"
#include "DownloadItemManager.h"
#include "DownloadJet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDownloadJet::CDownloadJet()
{

}

CDownloadJet::~CDownloadJet()
{

}



void CDownloadJet::SetURL(LPCTSTR lpszDownloadUrl)
{
	m_strDownloadUrl = lpszDownloadUrl;
}

LPCTSTR CDownloadJet::GetHeaderInfo()
{
	return m_strHeader;
}

void CDownloadJet::SetRange(PBREAKPOSITION pBrkPos)
{
	m_pBrkPos = pBrkPos;
}

DWORD CDownloadJet::GetContentLength()
{
	return m_dwContentLength;
}



DWORD CDownloadJet::GetCurPos()
{
	return m_pBrkPos->iStart;
}

LPCTSTR CDownloadJet::GetDownloadBuf()
{
	return m_szReadBuf;
}

DWORD CDownloadJet::GetCurDownloadedSize()
{
	return m_iDownloadedBufSize;
}

PBREAKPOSITION CDownloadJet::GetRange()
{
	return m_pBrkPos;
}

BOOL CDownloadJet::IsSupportResume()
{
	return m_bSupportResume;
}



// ���ó�ʱ
void CDownloadJet::SetTimeout(DWORD dwSendTimeout, DWORD dwReceiveTimeout, DWORD dwConnectTimeout)
{
	if( dwSendTimeout > 0 )
		m_dwSendTimeout = dwSendTimeout;
	
	if( dwReceiveTimeout > 0 )
		m_dwReceiveTimeout = dwReceiveTimeout;

	if( dwConnectTimeout > 0 )
		m_dwConnectTimeout = dwConnectTimeout;


}

// ����UserAgent
void CDownloadJet::SetUserAgent(LPCTSTR lpszUserAgent)
{
	m_strUserAgent = lpszUserAgent;	
	if( m_strUserAgent.IsEmpty())
		m_strUserAgent = _T("HttpDownload/2.0");
}

// ����Referer
void CDownloadJet::SetReferer(LPCTSTR lpszReferer)
{
	if( lpszReferer != NULL )
		m_strReferer = lpszReferer;
	else
		m_strReferer = _T("");
}

// �������ԵĻ���
// nRetryType = 0  ������					RETRY_TYPE_NONE
// nRetryType = 1  ����һ������				RETRY_TYPE_TIMES
// nRetryType = 2  ��Զ����(����������ѭ��)	RETRY_TYPE_ALWAYS
void CDownloadJet::SetRetry(UINT nRetryType, UINT nRetryDelay, UINT nRetryMax)
{
	m_nRetryType  = nRetryType;
	m_nRetryDelay = nRetryDelay;
	m_nRetryMax	  = nRetryMax;
	
	// ���һ��m_nRetryMax,���Ϊ0����Ϊȱʡֵ
	if( (RETRY_TYPE_TIMES == m_nRetryType) && (0 == m_nRetryMax) )
		m_nRetryMax = DEFAULT_RETRY_MAX;
}



// ����WWW��֤��Ϣ
void CDownloadJet::SetAuthorization(LPCTSTR lpszUsername, LPCTSTR lpszPassword, BOOL bAuthorization)
{
	if( bAuthorization && lpszUsername != NULL )
	{
		m_bAuthorization = TRUE;
		m_strUsername	 = lpszUsername;
		m_strPassword	 = lpszPassword;
	}
	else
	{
		m_bAuthorization = FALSE;
		m_strUsername	 = _T("");
		m_strPassword	 = _T("");
	}
}

void CDownloadJet::SetForceDownload(BOOL bForceDownload)
{
	m_bForceDownload = bForceDownload;
}




CString CDownloadJet::EncapsulateJetInfo(int iType, LPCTSTR lpszContent)
{
	CTime time;
	time = CTime::GetCurrentTime();

	CString strInfo;
	strInfo.Format("%d:%u:%s%c", iType, time, lpszContent, '\t');
	
	return strInfo;
}

void CDownloadJet::SendFailedMsg()
{
	m_pParent->JetFailed(this, EncapsulateJetInfo(STATE_MSG, NULL));
}

void CDownloadJet::SendRecvOKMsg()
{
	m_pParent->JetRecvOK(this, EncapsulateJetInfo(STATE_MSG, NULL));
}

void CDownloadJet::SendCompletedMsg()
{
	CString strMsg;
	strMsg = "Close.\r\n";

	m_pParent->JetCompleted(this, EncapsulateJetInfo(STATE_MSG, strMsg));
}

BOOL CDownloadJet::SendStartDownloadMsg()
{

	CString strMsg;
	strMsg = "Start to receive data!\r\n";

	return m_pParent->JetStartRecv(this, EncapsulateJetInfo(STATE_MSG, strMsg));
}

void CDownloadJet::SendHeaderInfoMsg(int iType)
{
	CString strMsg;
	strMsg = m_strHeader;

	m_pParent->JetGeneralInfo(this, EncapsulateJetInfo(iType, strMsg));
}

void CDownloadJet::SendResultFailedMsg()
{

	CString strMsg;
	strMsg = "Download task failed\r\n";
	m_pParent->JetResultFailed(this, EncapsulateJetInfo(ERROR_MSG, strMsg));
}

void CDownloadJet::SendCancelMsg()
{
	CString strMsg;
	strMsg.Format("User Cancel At %d\r\n", m_pBrkPos->iStart);

	m_pParent->JetCanceled(this, EncapsulateJetInfo(ERROR_MSG, strMsg));

}



int CDownloadJet::GetSocket()
{
	return m_hSocket;
}
