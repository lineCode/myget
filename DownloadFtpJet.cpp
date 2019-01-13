
// DownloadFtpJet.cpp : implementation file
//

#include "stdafx.h"
#include "MyGet.h"
#include "TE_Socket.h"
#include "SocksPacket.h"	// Socks Proxy Support
#include "DownloadItemManager.h"
#include "DownloadFtpJet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

JET_STATE_ENTRY CDownloadFtpJet::m_JetStatesTable[] = 
{
	///*none		*/{prv_Connected,	prv_Failed,	prv_Error},
	///*connected	*/{prv_SentUserName,prv_Failed,	prv_Error},
	///*user_name	*/{prv_SentPassword,prv_Failed,	prv_Error},
	///*password	*/{prv_SentRest,	prv_Failed,	prv_Error},
	///*rest_0	*/{prv_SentRest,	prv_Failed,	prv_Error},
	///*rest_1	*/{prv_SentType,	prv_Failed,	prv_Error},
	///*type_0	*/{prv_SentPasv,	prv_Failed,	prv_Error},
	///*pasv_0	*/{prv_GetList,		prv_Failed,	prv_Error},
	///*get_list	*/{prv_SentList,	prv_Failed,	prv_Error},
	///*list		*/{prv_SentType,	prv_Failed,	prv_Error},
	///*type_1	*/{prv_SentSize,	prv_Failed,	prv_Error},
	///*size		*/{prv_SentType,	prv_Failed,	prv_Error},
	///*type_2	*/{prv_SentPasv,	prv_Failed,	prv_Error},
	///*pasv_1	*/{prv_SentRest,	prv_Failed,	prv_Error},
	///*rest_2	*/{prv_SentRetr,	prv_Failed,	prv_Error},
	///*retr		*/{prv_SentRetr,	prv_Failed,	prv_Error},
	/*close		*/{NULL,			NULL,		NULL}
};

int CDownloadFtpJet::m_nSizeOfStatesTable = sizeof(m_JetStatesTable) / sizeof(JET_STATE_ENTRY);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDownloadFtpJet::CDownloadFtpJet(CDownloadItemManager *pParent)
{
	m_pParent = pParent;

	m_strDownloadUrl	= _T("");

	// ֹͣ����
	m_bStopDownload		= FALSE;

	// ǿ����������(�������е��ļ��Ƿ���Զ���ļ���ͬ)
	m_bForceDownload	= FALSE;

	// �Ƿ�֧�ֶϵ�����(�ٶ���֧��)
	m_bSupportResume	= FALSE;


	m_dwHeaderSize		= 0;		// HTTPЭ��ͷ�ĳ���
	m_strHeader			= _T("");	// HTTPЭ��ͷ

	// Referer
	m_strReferer		= _T("");
	
	// UserAgent
	m_strUserAgent		= _T("FtpDownload/2.0");

	// ��ʱTIMEOUT	���ӳ�ʱ�����ͳ�ʱ�����ճ�ʱ(��λ������)
	m_dwConnectTimeout	= DOWNLOAD_CONNECT_TIMEOUT;	
	m_dwReceiveTimeout	= DOWNLOAD_RECV_TIMEOUT;
	m_dwSendTimeout		= DOWNLOAD_SEND_TIMEOUT;

	// ���Ի���
	m_nRetryType		= RETRY_TYPE_NONE;	//��������(0:������ 1:����һ������ 2:��������)
	m_nRetryTimes		= 0;				//���Դ���
	m_nRetryDelay		= 0;				//�����ӳ�(��λ������)
	m_nRetryMax			= 0;				//����������

	// ������
	m_nErrorCount		= 0;			//�������
	m_strError			= _T("");		//������Ϣ


	// �Ƿ������֤ : Request-Header: Authorization
	m_bAuthorization	= FALSE;
	m_strUsername		= _T("");
	m_strPassword		= _T("");

	// �Ƿ�ʹ�ô��� 
	m_bProxy			= FALSE;
	m_strProxyServer	= _T("");
	m_nProxyPort		= 0;
	m_nProxyType		= PROXY_NONE;
	
	// �����Ƿ���Ҫ��֤: Request-Header: Proxy-Authorization
	m_bProxyAuthorization = FALSE;
	m_strProxyUsername 	= _T("");
	m_strProxyPassword	= _T("");

	// ���ع��������õı���
	m_strServer			= _T("");
	m_strObject			= _T("");
	//m_strFileName		= _T("");
	m_nPort				= DEFAULT_FTP_PORT ;
	
	// SOCKET �� BufSocket
	m_hSocket			= INVALID_SOCKET;

	m_pBrkPos			= NULL;
		
	m_JetStatus			= CDownloadFtpJet::none;

	const int INIT_RECV_BUFFER_SIZE = 4096;
	m_szBuffer = new char[INIT_RECV_BUFFER_SIZE];
	m_iBufferSize = INIT_RECV_BUFFER_SIZE;
	m_iDataLen = 0;

	m_bControlSocketReady = FALSE;
	m_bDataSocketReady = FALSE;
}

// ��������
CDownloadFtpJet::~CDownloadFtpJet()
{
}





// �������
void CDownloadFtpJet::prv_StartDownload()
{
	m_bStopDownload	  = FALSE;
	m_nRetryTimes	  = 0;

	// ����Ҫ���ص�URL�Ƿ�Ϊ��
	m_strDownloadUrl.TrimLeft();
	m_strDownloadUrl.TrimRight();
	if( m_strDownloadUrl.IsEmpty() )
	{
		//return DOWNLOAD_RESULT_FAIL;
		m_JetStatus = CDownloadFtpJet::close;
		return;
	}

	// ����Ҫ���ص�URL�Ƿ���Ч
	if ( !ParseURL(m_strDownloadUrl, m_strServer, m_strObject, m_nPort))
	{
		// ��ǰ�����"http://"����
		m_strDownloadUrl = _T("http://") + m_strDownloadUrl;
		if ( !ParseURL(m_strDownloadUrl,m_strServer, m_strObject, m_nPort) )
		{
			TRACE(_T("Failed to parse the URL: %s\n"), m_strDownloadUrl);

			//return DOWNLOAD_RESULT_FAIL;
			m_JetStatus = CDownloadFtpJet::close;
			return;
		}
	}


//	prv_c
}
	



// ��URL�����ֳ�Server��Object��
BOOL CDownloadFtpJet::ParseURL(LPCTSTR lpszURL, CString &strServer, CString &strObject,USHORT& nPort)
{
	CString strURL(lpszURL);
	strURL.TrimLeft();
	strURL.TrimRight();
	
	// �������
	strServer = _T("");
	strObject = _T("");
	nPort	  = 0;

	int nPos = strURL.Find("://");
	if( nPos == -1 )
		return FALSE;

	// ��һ����֤�Ƿ�Ϊftp://
	CString strTemp = strURL.Left( nPos+lstrlen("://") );
	strTemp.MakeLower();
	if( strTemp.Compare("ftp://") != 0 )
		return FALSE;

	strURL = strURL.Mid( strTemp.GetLength() );
	nPos = strURL.Find('/');
	if ( nPos == -1 )
		return FALSE;

	strObject = strURL.Mid(nPos);
	strTemp   = strURL.Left(nPos);
	
	///////////////////////////////////////////////////////////////
	/// ע�⣺��û�п���URL�����û����Ϳ�������κ������#������
	/// ���磺http://abc@def:www.yahoo.com:81/index.html#link1
	/// 
	//////////////////////////////////////////////////////////////

	// �����Ƿ��ж˿ں�
	nPos = strTemp.Find(":");
	if( nPos == -1 )
	{
		strServer = strTemp;
		nPort	  = DEFAULT_FTP_PORT;
	}
	else
	{
		strServer = strTemp.Left( nPos );
		strTemp	  = strTemp.Mid( nPos+1 );
		nPort	  = (USHORT)_ttoi((LPCTSTR)strTemp);
	}
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CDownloadItemManagerThread message handlers




void CDownloadFtpJet::StartDownload()
{
	DownloadHandler(0, 0);
}


// ֹͣ����
void CDownloadFtpJet::StopDownload()
{
	m_bStopDownload = TRUE;

	//closesocket(m_hSocket);
	prv_StopDownload();
}


// ���ô���������֤��ʽ
void CDownloadFtpJet::SetProxy(LPCTSTR lpszProxyServer, USHORT nProxyPort, BOOL bProxy, BOOL bProxyAuthorization, LPCTSTR lpszProxyUsername, LPCTSTR lpszProxyPassword,UINT nProxyType /*= PROXY_HTTPGET*/)
{
	if( bProxy && lpszProxyServer != NULL)
	{
		m_bProxy			= TRUE;
		m_strProxyServer	= lpszProxyServer;
		m_nProxyPort		= nProxyPort;
		m_nProxyType		= nProxyType;

		if( bProxyAuthorization && lpszProxyUsername != NULL)
		{
			m_bProxyAuthorization	= TRUE;
			m_strProxyUsername		= lpszProxyUsername;
			m_strProxyPassword		= lpszProxyPassword;
		}
		else
		{
			m_bProxyAuthorization	= FALSE;
			m_strProxyUsername		= _T("");
			m_strProxyPassword		= _T("");
		}
	}
	else
	{
		m_bProxy				= FALSE;
		m_bProxyAuthorization	= FALSE;
		m_nProxyPort			= 0;
		m_nProxyType			= PROXY_NONE;
		m_strProxyServer		= _T("");
		m_strProxyUsername		= _T("");
		m_strProxyPassword		= _T("");
	}
}

enum CDownloadFtpJet::REPLY_STATUS CDownloadFtpJet::prv_CheckReply(WPARAM wParam, LPARAM lParam)
{
	enum CDownloadFtpJet::REPLY_STATUS rsReplyStatus = nUncompletedCommand;

	char byBuffer[1024];

	if (wParam != m_hSocket && WSAGETSELECTEVENT(lParam) == FD_CONNECT)
	{
		return nSucceed;
	}

	memset(byBuffer, 0, sizeof(byBuffer));

	int iRet = recv(wParam, byBuffer, sizeof(byBuffer), 0);

	if (iRet <= 0)
	{
		if (iRet == 0 && wParam == m_hDataSocket)
		{
			rsReplyStatus = nSucceed;
			m_bDataSocketReady = TRUE;
		}
		else
		{
			rsReplyStatus = nError;
		}
	}
	else
	{
		if (m_iBufferSize < m_iDataLen + iRet)
		{
			int iNewSize = m_iBufferSize;
			while (iNewSize < m_iDataLen + iRet)
			{
				iNewSize *= 2;
			}

			char* tmpBuf = new char[iNewSize];
			memcpy(tmpBuf, m_szBuffer, m_iDataLen);

			delete[] m_szBuffer;
			m_szBuffer = tmpBuf;
		}
	
		memcpy(m_szBuffer + m_iDataLen, byBuffer, iRet);
		m_iDataLen += iRet;

		if ((m_JetStatus == retr || m_JetStatus == list) && wParam == m_hDataSocket)
		{
			rsReplyStatus = nSucceed;
		}
		else
		{
			if (prv_GetReplyEndPosition(m_szBuffer, m_iDataLen) > 0)
			{
				rsReplyStatus = nSucceed;
			}
		}
	}

	return rsReplyStatus;
}

int CDownloadFtpJet::prv_GetReplyEndPosition(char *pszBuf, int iBufLen)
{
	int iCommandEndPosition = 0;
	if (iBufLen > 4)
	{
		if (pszBuf[3] != '-')
		{
			for (int i = 3; i < iBufLen; i ++)
			{
				if (pszBuf[i] == 0x0a && pszBuf[i - 1] == 0x0d)
				{
					iCommandEndPosition = i;
					break;
				}

			}
		}
	}

	return iCommandEndPosition;
};

int CDownloadFtpJet::DownloadHandler(WPARAM wParam, LPARAM lParam)
{

	int iSetEvent = 0;
	enum CDownloadFtpJet::REPLY_STATUS rsReplyStatus = nError;

	if (wParam == 0 && lParam == 0)
	{
		prv_ConnectServer(wParam);
		iSetEvent = FD_CONNECT | FD_CLOSE;
	}
	else if (WSAGETSELECTERROR(lParam))
	{
		m_JetStatus = CDownloadFtpJet::close;
		iSetEvent = 0;
	}
	else 
	{
		rsReplyStatus = prv_CheckReply(wParam, lParam);	

		if (rsReplyStatus != nUncompletedCommand)
		{
			JET_STATE_HANDLER fJetStateHandler = m_JetStatesTable[m_JetStatus].fStateHandlers[rsReplyStatus];
			(this->*fJetStateHandler)(wParam);
		}
		
	}


	if (m_JetStatus != CDownloadFtpJet::close)
	{
		Sleep(50);
		iSetEvent = FD_READ | FD_CLOSE;
	}
	else
	{
		//Ӧ����������Jet����
		prv_StopDownload();
	}

	return iSetEvent;
}




// ��������(��չ����)
int CDownloadFtpJet::prv_ConnectEx(SOCKET hSocket, char const * pszServer, int nPort,DWORD dwTimeout,BOOL fFixNtDNS /*= FALSE*/)
{
	/////////////////////////////////////////////////////////////////////////////
	SOCKADDR_IN sockAddr;
	ZeroMemory(&sockAddr,sizeof(sockAddr));

	sockAddr.sin_family			= AF_INET;
	sockAddr.sin_port			= htons((u_short)nPort);
	sockAddr.sin_addr.s_addr	= TE_GetIP(pszServer,fFixNtDNS);
	
	if (sockAddr.sin_addr.s_addr == INADDR_NONE)
	{
		TE_SetLastError( WSAGetLastError() );
		return (SOCKET_ERROR);
	}
	//////////////////////////////////////////////////////////////////////

	WSAConnect(hSocket, (SOCKADDR *)&sockAddr, sizeof(sockAddr), NULL, NULL, NULL, NULL);
	
	m_pParent->RegisterTimeout(this, dwTimeout);

	return 0;
}

void CDownloadFtpJet::Timeout()
{

}

void CDownloadFtpJet::prv_UnregisterTimeout()
{
	m_pParent->UnregisterTimeout(this);
}

int CDownloadFtpJet::prv_Send(SOCKET hSocket, const char *pszBuffer, int iBufferSize, DWORD dwTimeout)
{
	send(hSocket, pszBuffer, iBufferSize, 0);

	m_pParent->RegisterTimeout(this, dwTimeout);
	return 0;
}



void CDownloadFtpJet::prv_StopDownload()
{
	closesocket(m_hSocket);
	prv_UnregisterTimeout();
	m_pParent->RemoveDownloadEntry(this);

	//delete this;
}


void CDownloadFtpJet::prv_ConnectServer(SOCKET aSocket)
{
	// ����Ҫ���ص�URL�Ƿ�Ϊ��
	m_strDownloadUrl.TrimLeft();
	m_strDownloadUrl.TrimRight();
	if( m_strDownloadUrl.IsEmpty() )
	{
		//return DOWNLOAD_RESULT_FAIL;
		m_JetStatus = CDownloadFtpJet::close;
		return;
	}

	// ����Ҫ���ص�URL�Ƿ���Ч
	if ( !ParseURL(m_strDownloadUrl, m_strServer, m_strObject, m_nPort))
	{
		// ��ǰ�����"ftp://"����
		m_strDownloadUrl = _T("ftp://") + m_strDownloadUrl;
		if ( !ParseURL(m_strDownloadUrl,m_strServer, m_strObject, m_nPort) )
		{
			TRACE(_T("Failed to parse the URL: %s\n"), m_strDownloadUrl);

			//return DOWNLOAD_RESULT_FAIL;
			m_JetStatus = CDownloadFtpJet::close;
			return;
		}
	}

	//���Ƚ���һ��sokcet,Ȼ����뵽DownloadScheduler���첽select�С�
	closesocket(m_hSocket);
	m_hSocket = socket(AF_INET,SOCK_STREAM,0);

	m_pParent->AddDownloadEntry(this, m_hSocket);

	//���ӷ�����
	m_strHeader		= _T("");
	m_dwHeaderSize	= 0;

	m_strHeader.Format("%s%s:%d\r\n", "Connect to ", m_strServer, m_nPort);
	SendHeaderInfoMsg(COMMAND_MSG);

	//���ݲ�ͬ�Ĵ���ģʽ���ֱ��������
	switch( m_nProxyType )
	{
	case PROXY_NONE:
		prv_ConnectEx(m_hSocket, m_strServer, m_nPort, m_dwConnectTimeout, TRUE);
		break;
	//Ϊ�˼������Ŀǰ��ʱ��֧�ִ���ģʽ��
	default:
		m_JetStatus = CDownloadFtpJet::close;
		return;
		break;
	}


	m_JetStatus = CDownloadFtpJet::none;
	return;
}


void CDownloadFtpJet::prv_Connected(SOCKET aSocket)
{
	char szSendBuf[1024];

	m_iDataLen = 0;
	sprintf(szSendBuf, "USER %s\r\n", "anonymous");
	send(m_hSocket, szSendBuf, strlen(szSendBuf), 0);
	m_JetStatus = CDownloadFtpJet::connected;
};

void CDownloadFtpJet::prv_SentUserName(SOCKET aSocket)
{
	char szSendBuf[1024];

	m_iDataLen = 0;
	sprintf(szSendBuf, "PASS %s\r\n", "a@b.com");
	send(m_hSocket, szSendBuf, strlen(szSendBuf), 0);
	m_JetStatus = CDownloadFtpJet::user_name;
};

void CDownloadFtpJet::prv_SentPassword(SOCKET aSocket)
{
	char szSendBuf[1024];

	m_iDataLen = 0;
	sprintf(szSendBuf, "REST 100\r\n");
	send(m_hSocket, szSendBuf, strlen(szSendBuf), 0);
	m_JetStatus = CDownloadFtpJet::password;

};

void CDownloadFtpJet::prv_SentRest(SOCKET aSocket)
{
	char szSendBuf[1024];

	m_iDataLen = 0;

	switch(m_JetStatus)
	{
	case CDownloadFtpJet::password:
		sprintf(szSendBuf, "REST 0\r\n");
		m_JetStatus = CDownloadFtpJet::rest_0;
		break;
		
	case CDownloadFtpJet::rest_0:
 		sprintf(szSendBuf, "TYPE A\r\n");
		m_JetStatus = CDownloadFtpJet::rest_1;
		break;

	case CDownloadFtpJet::pasv_1:
		m_JetStatus = CDownloadFtpJet::rest_2;
		break;

	default:
		m_JetStatus = CDownloadFtpJet::close;
		return;
		break;
	}

	send(m_hSocket, szSendBuf, strlen(szSendBuf), 0);
	
};

void CDownloadFtpJet::prv_SentType(SOCKET aSocket)
{
	char szSendBuf[1024];

	m_iDataLen = 0;
	switch(m_JetStatus)
	{
	case CDownloadFtpJet::rest_1:
		sprintf(szSendBuf, "PASV\r\n");
		m_JetStatus = CDownloadFtpJet::type_0;
		break;

	case CDownloadFtpJet::list:
		sprintf(szSendBuf, "SIZE %s\r\n", m_strObject);
		m_JetStatus = CDownloadFtpJet::type_1;
		break;

	case CDownloadFtpJet::size:
		sprintf(szSendBuf, "PASV\r\n");
		m_JetStatus = CDownloadFtpJet::type_2;
		break;

	default:
		m_JetStatus = CDownloadFtpJet::close;
		return;
 		break;

	}

	send(m_hSocket, szSendBuf, strlen(szSendBuf), 0);
};

void CDownloadFtpJet::prv_SentPasv(SOCKET aSocket)
{
	char szIP[20];
	UINT uiPort;
	m_iDataLen = 0;

	GetIPandPort(szIP, &uiPort);

	m_hDataSocket = socket(PF_INET, SOCK_STREAM, 0);

	m_pParent->AddDownloadEntry(this, m_hDataSocket);
	prv_ConnectEx(m_hDataSocket, szIP, uiPort, m_dwConnectTimeout, TRUE);


	m_JetStatus = CDownloadFtpJet::pasv_0;
};

BOOL CDownloadFtpJet::GetIPandPort(char *pszIP, UINT *puiPort)
{
	char  strPort[10], strP[10];
	char  *pDest, *pCur;
	int	  result1, result2;
 
	pDest = strchr(m_szBuffer, '(' );
	pszIP[0] = '\0';
	memset(strPort, 0, sizeof(strPort));
	pCur = pDest+1;
	pDest = pCur;

	for(int i = 0; i < 4; i++)
	{
		while(*(pCur++) != ',');
	 	strncat(pszIP, pDest, pCur-pDest-1);
		while(i < 3)
		{
 			strcat(pszIP, ".");
			break;
		}
		pDest = pCur;
	}
 
	int i = 0;
	for(i=0; i < 2; i++)
	{
		while(*(pCur) != ',' && *(pCur) != ')')
		{
			pCur++;
		}


		if(i == 0)
		{
			memset(strP, 0, sizeof(strP));
			strncpy(strP, pDest, pCur - pDest);
			result1 = atoi(strP)*256;
		}

 		if( i == 1)
		{
			memset(strP, 0, sizeof(strP));
			strncpy(strP, pDest, pCur - pDest);
			result2 = atoi(strP);
		}

		pCur++;
		pDest = pCur;
	}
	
	*puiPort = (unsigned short)(result1 + result2);

	return TRUE;
}

void CDownloadFtpJet::prv_GetList(SOCKET aSocket)
{
	char szSendBuf[1024];
	m_iDataLen = 0;

	sprintf(szSendBuf, "LIST %s\r\n", m_strObject);
	send(m_hSocket, szSendBuf, strlen(szSendBuf), 0);

	m_JetStatus = CDownloadFtpJet::get_list;
};

void CDownloadFtpJet::prv_SentList(SOCKET aSocket)
{

	if (aSocket == m_hSocket)
	{
		m_bControlSocketReady = TRUE;
		m_iDataLen = 0;

	}
	else if (aSocket == m_hDataSocket)
	{

	}

	if (m_bControlSocketReady && m_bDataSocketReady)
	{
		char szSendBuf[1024];
		m_iDataLen = 0;
	
		m_pParent->RemoveDownloadEntry(this);
		sprintf(szSendBuf, "TYPE I\r\n");
		send(m_hSocket, szSendBuf, strlen(szSendBuf), 0);
		m_JetStatus = CDownloadFtpJet::list;
	}
};

void CDownloadFtpJet::prv_SentSize(SOCKET aSocket)
{
	char szSendBuf[1024];
	m_iDataLen = 0;

	sprintf(szSendBuf, "SIZE %s\r\n", m_strObject);

	send(m_hSocket, szSendBuf, strlen(szSendBuf), 0);
	m_JetStatus = CDownloadFtpJet::size;

};

void CDownloadFtpJet::prv_SentRetr(SOCKET aSocket)
{

};

void CDownloadFtpJet::prv_Failed(SOCKET aSocket)
{

};

void CDownloadFtpJet::prv_Error(SOCKET aSocket)
{
	m_JetStatus = CDownloadFtpJet::close;
};

BOOL CDownloadFtpJet::OwnSocket(SOCKET aSocket)
{
	if (m_hDataSocket == aSocket)
	{
		return TRUE;
	}
	else
	{
		return CDownloadJet::OwnSocket(aSocket);
	}
}
