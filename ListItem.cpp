
#include "stdafx.h"
#include "ListItem.h"

#include "JcdFile.h"
#include "ItemHeader.h"
#include "MyGet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CListItem::CListItem() : CItemHeader(ITEM_CLASS_LISTITEM)
{
	prv_InitDefault();
}



int CListItem::GetFileTotalLengthByString(LPTSTR pBuf, int iMaxBufLength)
{
	//ע��iMaxBufLengthû�н��м�顣
	if (m_iSizeOfFile != -1)
	{
		itoa(m_iSizeOfFile, pBuf, 10);
		
		return strlen(pBuf);
	}
	else
	{
		pBuf[0] = '\0';
		return 0;
	}
	
}

LPCTSTR CListItem::GetComment()
{
	return m_strComment;
}



int CListItem::GetItemStatusIconIndex()
{
	return GetStatus();
}




LPCTSTR CListItem::GetURL()
{
	return m_strURL;
}

LPCTSTR CListItem::GetREF()
{
	return m_strREF;
}


int CListItem::IsSupportResumed()
{
	if (m_iAttribute & ATTR_EXCUTED)
	{
		if (m_iAttribute & ATTR_SUPPORTRESUME)
		{
			return SUPPORTRESUME;
		}
		else
		{
			return UNSUPPORTRESUME;
		}
	}
	else
	{
		//Unknow because the downloading is not excuted before.
		return UNKNOWN;
	}

	return UNKNOWN;
}

LPCTSTR CListItem::GetDownloadedFolder()
{
	return m_strDownloadFolder;
}

time_t CListItem::GetFileTime()
{
	return (time_t)m_iFileCreateTime;
}

time_t CListItem::GetLocalFileCreatedTime()
{
	return (time_t)m_iItemCreateTime;
}

time_t CListItem::GetLocalFileCompletedTime()
{
	return (time_t)m_iItemCompleteTime;
}

int CListItem::GetFileDownloadedSize()
{
	int iFileSize = 0;

	for (int i = 0; i < this->GetCountOfBreaks(); i ++)
	{
		iFileSize += m_arBrkPosArray[i]->iEnd 
					- m_arBrkPosArray[i]->iStart; 
	}

	return iFileSize;
}

int CListItem::GetDownloadingTimeCosted()
{
	return m_iDownloadingTimeCosted;
}

int CListItem::GetFileSize()
{
	return m_iSizeOfFile;
}



BOOL CListItem::IsCompleted()
{
	return (m_iIsFileCompleted == COMPLETED);
}



LPCTSTR CListItem::GetPasswd()
{
	return m_strPasswd;
}

LPCTSTR CListItem::GetRename()
{
	if (!m_strRename.IsEmpty())
	{
		return m_strRename;
	}
	else
	{
		return GetRenameByURL(m_strURL);
	}
}

LPCTSTR CListItem::GetUserName()
{
	return m_strUserName;
}

int CListItem::GetCountOfSubstitutedURLs()
{
	return m_arSubStitutedUrl.GetSize();
}

LPCTSTR CListItem::GetSubstitutedURLs(int iIndex)
{
	if (iIndex < GetCountOfSubstitutedURLs())
	{
		return m_arSubStitutedUrl[iIndex]->m_strSubstitutedURL;
	}

	return NULL;
}



int CListItem::GetStatus()
{
	switch (m_iIsFileCompleted)
	{
	case DOWNLOAD_ERR:
		return ITEM_STATUS_DOWNLOAD_ERROR;
	case COMPLETED:
		return ITEM_STATUS_COMPLETED;
	case DOWNLOADING:
		return ITEM_STATUS_DOWNLOADING;
	case UNCOMPLETED:
		switch (m_wStartState)
		{
		case START_MANUAL:
			return ITEM_STATUS_PAUSED;
		case START_RIGHTNOW:
			return ITEM_STATUS_READY;
		case START_SCHEDULED:
			return ITEM_STATUS_SCHEDULED;
		}
	}

	return -1;
}

void CListItem::prv_FreeMEM()
{
	//free substituted URLs
	PSUBSTITUTEDURL pSubsUrl;
	int i;
	for (i =0; i < GetCountOfSubstitutedURLs(); i ++)
	{
		pSubsUrl = m_arSubStitutedUrl[i];

		delete pSubsUrl;
	}

	m_arSubStitutedUrl.RemoveAll();

	//free Breaks;
	PBREAKPOSITION pBreakPos;

	for (i = 0; i < GetCountOfBreaks(); i ++)
	{
		pBreakPos = m_arBrkPosArray[i];
		delete pBreakPos;
	}

	m_arBrkPosArray.RemoveAll();

}



int CListItem::GetCountOfBreaks()
{
	return m_arBrkPosArray.GetSize();
}

BOOL CListItem::LoadFromFile(CJCDFile *pJCDFile)
{
	CItemHeader::LoadFromFile(pJCDFile);
	//����URL.
	JCD_LOAD_STR(m_strURL);

	//�������ԡ�
	JCD_LOAD_INT(m_iAttribute);

	//�����¼����������
	JCD_LOAD_STR(m_strUserName);


	//��������
	JCD_LOAD_STR(m_strPasswd);

	//����������ɺ�Ӧ������TreeItem����𣬼�TreeItem��ID
	JCD_LOAD_INT(m_iDownloadedToCategory);

	//������������
	JCD_LOAD_STR(m_strRename);

	//�����ļ�ͬʱ���߳����ص��߳���
	JCD_LOAD_INT(m_iConcurrentThreadNumber);

	//��������Ŀ¼
	JCD_LOAD_STR(m_strDownloadFolder);

	//����ע��
	JCD_LOAD_STR(m_strComment);

	//����Http Proxy����
	JCD_LOAD_WORD(m_wHttpProxyIndex);

	//����ʼ״̬
	JCD_LOAD_WORD(m_wStartState);

	//����4�ֽڵ��ļ��Ƿ����ؽ�������
	JCD_LOAD_INT(m_iIsFileCompleted);

	//�����ļ����Ȳ���
	JCD_LOAD_INT(m_iSizeOfFile);

	//����1�ֽڵ�Unknown4����
	JCD_LOAD_BYTE(m_pbUnknown4);
	

	//����4�ֽڵ��ļ�����ʱ��
	JCD_LOAD_INT(m_iFileCreateTime);

	//����4�ֽڵ����ص�Item�Ĵ���ʱ��
	JCD_LOAD_INT(m_iItemCreateTime);

	//����4�ֽڵ����ص�Item�Ľ���ʱ��
	JCD_LOAD_INT(m_iItemCompleteTime);

	//��������
	JCD_LOAD_STR(m_strREF);

	//����4�ֽڵ����ص�ʱ������
	JCD_LOAD_INT(m_iDownloadingTimeCosted);

	//����Ftp Proxy����
	JCD_LOAD_WORD(m_wFtpProxyIndex);

	//����ɹ������URL.
	int iSumOfSubstitutedURLs;
	JCD_LOAD_INT(iSumOfSubstitutedURLs);

	PSUBSTITUTEDURL pSubsUrl;
	int i;
	for (i = 0; i < iSumOfSubstitutedURLs; i ++)
	{	
		pSubsUrl = new SUBSTITUTEDURL;

		JCD_LOAD_STR(pSubsUrl->m_strSubstitutedURL);

		//����4�ֽڵĿհ�����
		//Ϊ�˼򻯲�������ʱ�����4�ֽڵ�BYTE���鿴��һ��������
		JCD_LOAD_INT(pSubsUrl->pbUnknown);

		m_arSubStitutedUrl.Add(pSubsUrl);
	}


	//����4�ֽڵ��Ƿ��Ѿ���������
	int iBreaksCount;
	JCD_LOAD_INT(iBreaksCount);

	PBREAKPOSITION pBreak;
	for (i = 0; i < iBreaksCount; i ++)
	{
		pBreak = new BREAKPOSITION;

		JCD_LOAD_INT(pBreak->iStart);
		JCD_LOAD_INT(pBreak->iEnd);

		m_arBrkPosArray.Add(pBreak);
	}

	return TRUE;
}

PBREAKPOSITION CListItem::GetBreakPos(DWORD dwIndex)
{
	return m_arBrkPosArray[dwIndex];
}

BOOL CListItem::SaveToFile(CJCDFile *pJCDFile)
{
	CItemHeader::SaveToFile(pJCDFile);
	//����URL.
	JCD_SAVE_STR(m_strURL);

	//�������ԡ�
	JCD_SAVE_INT(m_iAttribute);

	//�����¼����������
	JCD_SAVE_STR(m_strUserName);


	//��������
	JCD_SAVE_STR(m_strPasswd);

	//����������ɺ�Ӧ������TreeItem����𣬼�TreeItem��ID
	JCD_SAVE_INT(m_iDownloadedToCategory);

	//������������
	JCD_SAVE_STR(m_strRename);

	//�����ļ�ͬʱ���߳����ص��߳���
	JCD_SAVE_INT(m_iConcurrentThreadNumber);

	//��������Ŀ¼
	JCD_SAVE_STR(m_strDownloadFolder);

	//����ע��
	JCD_SAVE_STR(m_strComment);

	//����Http Proxy����
	JCD_SAVE_WORD(m_wHttpProxyIndex);

	//����ʼ״̬
	JCD_SAVE_WORD(m_wStartState);

	//����4�ֽڵ��ļ��Ƿ����ؽ�������
	JCD_SAVE_INT(m_iIsFileCompleted);

	//�����ļ����Ȳ���
	JCD_SAVE_INT(m_iSizeOfFile);

	//����1�ֽڵ�Unknown4����
	JCD_SAVE_BYTE(m_pbUnknown4);
	

	//����4�ֽڵ��ļ�����ʱ��
	JCD_SAVE_INT(m_iFileCreateTime);

	//����4�ֽڵ����ص�Item�Ĵ���ʱ��
	JCD_SAVE_INT(m_iItemCreateTime);

	//����4�ֽڵ����ص�Item�Ľ���ʱ��
	JCD_SAVE_INT(m_iItemCompleteTime);

	//��������
	JCD_SAVE_STR(m_strREF);

	//����4�ֽڵ����ص�ʱ������
	JCD_SAVE_INT(m_iDownloadingTimeCosted);

	//����Ftp Proxy����
	JCD_SAVE_WORD(m_wFtpProxyIndex);

	//����ɹ������URL.
	int iSumOfSubstitutedURLs = GetCountOfSubstitutedURLs();
	JCD_SAVE_INT(iSumOfSubstitutedURLs);

	PSUBSTITUTEDURL pSubsUrl;
	int i;
	for (i = 0; i < iSumOfSubstitutedURLs; i ++)
	{	
		pSubsUrl = m_arSubStitutedUrl[i];

		JCD_SAVE_STR(pSubsUrl->m_strSubstitutedURL);

		//����4�ֽڵĿհ�����
		//Ϊ�˼򻯲�������ʱ�����4�ֽڵ�BYTE���鿴��һ��������
		JCD_SAVE_INT(pSubsUrl->pbUnknown);

	}


	//����4�ֽڵ��Ƿ��Ѿ���������
	int iBreaksCount = GetCountOfBreaks();
	JCD_SAVE_INT(iBreaksCount);

	PBREAKPOSITION pBreak;
	for (i = 0; i < iBreaksCount; i ++)
	{
		pBreak = m_arBrkPosArray[i];

		JCD_SAVE_INT(pBreak->iStart);
		JCD_SAVE_INT(pBreak->iEnd);
	}

	return TRUE;
}

BOOL CListItem::IsAttrOn(int iAttrIndex)
{
	//Now, we only ensure the low 4 bits.
	BYTE bAttr = (BYTE)m_iAttribute;

	bAttr &= 0x0F;

	return (bAttr & iAttrIndex) ? TRUE : FALSE;

}

int CListItem::GetDownloadedToCategory()
{
	return m_iDownloadedToCategory;
}

int CListItem::GetSplitsCount()
{
	return m_iConcurrentThreadNumber;
}

WORD CListItem::GetStartState()
{
	return m_wStartState;
}

WORD CListItem::GetFtpProxyIndex()
{
	return m_wFtpProxyIndex;
}

WORD CListItem::GetHttpProxyIndex()
{
	return m_wHttpProxyIndex;
}

BOOL CListItem::prv_InitDefault()
{
	//CRegProcess RegProcess;
	CAppRegs *pAppRegs = ((CMyGetApp *)AfxGetApp())->GetAppRegs();

	m_iAttribute = 0;
	m_bDirty = TRUE;	
	//Fill Refer
	/*
	LPSTR lpszRefer = RegProcess.GetStr(REG_DOWNLOAD_DEF_REFERER);
	
	if (lpszRefer)
	{
		SetREF(lpszRefer);
		
		delete lpszRefer;
		lpszRefer = NULL;
	}
	*/

	LPSTR lpszRefer;
	pAppRegs->GetVal(REG_DOWNLOADDEFAULT_REFERER, &lpszRefer);
	
	SetREF(lpszRefer);
	
	
	//Fill DownloadToCategory
	//SetDownloadedToCategory(RegProcess.GetDWORD(REG_DOWNLOAD_DEF_CATEGORY));
	DWORD dwDownloadDefaultCategory;
	pAppRegs->GetVal(REG_DOWNLOADDEFAULT_SERVER, &dwDownloadDefaultCategory);
	SetDownloadedToCategory(dwDownloadDefaultCategory);

	//Fill Downloaded Path
	/*
	LPSTR lpszPath = RegProcess.GetStr(REG_DOWNLOAD_DEF_PATH);

	if (lpszPath)
	{
		SetDownloadedFolder(lpszPath);
		delete lpszPath;
		lpszPath = NULL;
	}*/
	LPSTR lpszPath;
	pAppRegs->GetVal(REG_DOWNLOADDEFAULT_PATH, &lpszPath);
	SetDownloadedFolder(lpszPath);


	//Fill split;
	//SetSplitsCount(RegProcess.GetDWORD(REG_DOWNLOAD_DEF_SPLIT));
	DWORD dwDownloadDefaultSplit;
	pAppRegs->GetVal(REG_DOWNLOADDEFAULT_SPLIT, &dwDownloadDefaultSplit);
	SetSplitsCount(dwDownloadDefaultSplit);

	//Fill "Mirror and Ftp seach"
	SetAttr(ATTR_NOTUSEMIRROR, !pAppRegs->IsFlagOn(REG_FLAG_GENERAL_EXT_ENABLE_MIRROE_AND_FTP_SEARCH));

	//Fill Login
//	SetAttr(ATTR_NEEDLOGIN, (RegProcess.GetDWORD(REG_DOWNLOAD_DEF_LOGIN) == ATTR_NEEDLOGIN));
	DWORD dwDownloadDefaultLogin;
	pAppRegs->GetVal(REG_DOWNLOADDEFAULT_LOGIN, &dwDownloadDefaultLogin);
	SetAttr(ATTR_NEEDLOGIN, dwDownloadDefaultSplit == ATTR_NEEDLOGIN);

	//Fill UserName
	/*
	LPTSTR lpszUserName = RegProcess.GetStr(REG_DOWNLOAD_DEF_USERNAME);
	
	if (lpszUserName)
	{
		SetUserName(lpszUserName);

		delete lpszUserName;
		lpszUserName = NULL;
	}
*/
	LPSTR lpszUserName;
	pAppRegs->GetVal(REG_DOWNLOADDEFAULT_USERNAME, &lpszUserName);
	SetUserName(lpszUserName);

	//Fill Password
/*	LPTSTR lpszPassword = RegProcess.GetStr(REG_DOWNLOAD_DEF_PASSWORD);

	if (lpszPassword)
	{
		SetPassword(lpszPassword);

		delete lpszPassword;
		lpszPassword = NULL;
	}
*/
	LPSTR lpszPassword;
	pAppRegs->GetVal(REG_DOWNLOADDEFAULT_PASSWORD, &lpszPassword);
	SetPassword(lpszPassword);
	
	//Fill Start
//	SetStartState((WORD)RegProcess.GetDWORD(REG_DOWNLOAD_DEF_START));

	DWORD dwDownloadDefaultStart;
	pAppRegs->GetVal(REG_DOWNLOADDEFAULT_START, &dwDownloadDefaultStart);
	SetStartState((WORD)dwDownloadDefaultStart);

	//Fill Http and Ftp Proxy CBX
//	SetFtpProxyIndex((WORD)RegProcess.GetDWORD(REG_PROXY_DEFAULT_FOR_FTP));
	DWORD dwDownloadDefaultProxyForFtp;
	pAppRegs->GetVal(REG_PROXY_DEFAULTFORFTP, &dwDownloadDefaultProxyForFtp);
	SetFtpProxyIndex((WORD)dwDownloadDefaultProxyForFtp);

	//SetHttpProxyIndex((WORD)RegProcess.GetDWORD(REG_PROXY_DEFAULT_FOR_HTTP));
	DWORD dwDownloadDefaultProxyForHttp;
	pAppRegs->GetVal(REG_PROXY_DEFAULTFORHTTP, &dwDownloadDefaultProxyForHttp);
	SetHttpProxyIndex((WORD)dwDownloadDefaultProxyForHttp);

	//Fill File complete state
	m_iIsFileCompleted = UNCOMPLETED;

	//Fill File size
	m_iSizeOfFile = -1;

	//Fill 1 byte unknown
	m_pbUnknown4[0] = 0;

	//Fill 3 file relevant time
	m_iFileCreateTime = m_iItemCompleteTime = m_iItemCompleteTime = -1;

	//Fill Donwload costing
	m_iDownloadingTimeCosted = 0;

	m_arSubStitutedUrl.RemoveAll();
	m_arBrkPosArray.RemoveAll();

	m_arJetsInfo.RemoveAll();
	m_iRetries = 0;

	return TRUE;
}

void CListItem::SetAttr(int iAttrIndex, BOOL bIsOn)
{
	if (bIsOn)
	{
		m_iAttribute |= iAttrIndex;	
	}
	else
	{
		DWORD dwMask = 0XFFFFFFFF - iAttrIndex;
		
		m_iAttribute &= dwMask;
	}

}

void CListItem::SetDownloadedToCategory(int iCategoryID)
{
	m_iDownloadedToCategory = iCategoryID;
}

void CListItem::SetDownloadedFolder(LPCTSTR lpszFolder)
{
	m_strDownloadFolder = lpszFolder;
}

void CListItem::SetSplitsCount(int iSplitsCount)
{
	m_iConcurrentThreadNumber = iSplitsCount;
}	

void CListItem::SetUserName(LPCTSTR lpszUserName)
{
	m_strUserName = lpszUserName;
}

void CListItem::SetPassword(LPCTSTR lpszPassword)
{
	m_strPasswd = lpszPassword;
}

void CListItem::SetStartState(WORD wStartState)
{
	m_wStartState = wStartState;
}

void CListItem::SetFtpProxyIndex(WORD wIndex)
{
	m_wFtpProxyIndex = wIndex;
}

void CListItem::SetHttpProxyIndex(WORD wIndex)
{
	m_wHttpProxyIndex = wIndex;
}

void CListItem::SetUrl(LPCTSTR lpszUrl)
{
	m_strURL = lpszUrl;
}

void CListItem::SetREF(LPCTSTR lpszRefer)
{
	m_strREF = lpszRefer;
}

void CListItem::ClearSubstitutedURLs()
{
	PSUBSTITUTEDURL pSubsUrl;

	for (int i = 0; i < GetCountOfSubstitutedURLs(); i ++)
	{
		pSubsUrl = m_arSubStitutedUrl[i];

		delete pSubsUrl;
	}

	m_arSubStitutedUrl.RemoveAll();
}

void CListItem::AddSubstitutedURL(LPCTSTR lpszSubUrl, int iUnknown)
{
	if (m_arBrkPosArray.GetSize() < MAX_SIM_DOWNLOAD_JETS)
	{
		PSUBSTITUTEDURL pSubsUrl = new SUBSTITUTEDURL;

		pSubsUrl->m_strSubstitutedURL = lpszSubUrl;
		*(int *)pSubsUrl->pbUnknown = iUnknown;

		m_arSubStitutedUrl.Add(pSubsUrl);
	}
}



void CListItem::SetRename(LPCTSTR lpszRename)
{
	m_strRename = lpszRename;
}

void CListItem::SetComment(LPCTSTR lpszComment)
{
	m_strComment = lpszComment;
}


void CListItem::SetItemCreateTime(time_t tItemCreateTime)
{
	m_iItemCreateTime = tItemCreateTime;
}

LPCTSTR CListItem::GetRenameByURL(LPCTSTR lpszUrl)
{
	if (!lpszUrl
		|| strlen(lpszUrl) == 0)
	{
		return "";
	}
	
	CString strUrl = lpszUrl;
	
	int iPos = -1;
	
	strUrl.MakeLower();
	
	iPos = strUrl.Find("http://");
	
	//first try to find "http://" or "ftp://"
	if (iPos == -1)
	{
		iPos = strUrl.Find("ftp://");
		
		if (iPos == -1)
		{
			return NULL;
		}
		else
		{
			iPos = strlen("ftp://");
		}
	}
	else
	{
		iPos = strlen("http://");
	}
	
	//If no more text contained in the URL, or the next char is '\'(illegal), just retrun;
	if (iPos == strUrl.GetLength()
		|| strUrl.GetAt(iPos) == TCHAR('/'))
	{
		return NULL;
	}
	
	int iLastBSlashPos;
	iLastBSlashPos = strUrl.ReverseFind('/');
	
	//if the remain text does not contain '/' or the last char is '/'
	//return "index.htm"
	if (iLastBSlashPos == strUrl.GetLength() - 1
		|| iLastBSlashPos < iPos)
	{
		return "index.htm";
	}
	else
	{
		//Now we can make the rename now.
		return lpszUrl + iLastBSlashPos + 1;
	}
};

void CListItem::ClearJetInfo()
{
	m_arJetsInfo.RemoveAll();
}

void CListItem::AddJetInfo(int iIndex, LPCTSTR lpszJetInfo)
{
	if (iIndex >= m_arJetsInfo.GetSize())
	{
		m_arJetsInfo.SetSize(iIndex + 1);
	}

	m_arJetsInfo[iIndex] += lpszJetInfo;

	CString strTmp;
	strTmp = m_arJetsInfo[iIndex];
}

CObject * CListItem::GetBrkPosArray()
{
	return (CObject *)&m_arBrkPosArray;
}

void CListItem::SetFileSize(DWORD dwFileSize)
{
	m_iSizeOfFile = dwFileSize;
}

void CListItem::SetFileStatus(int iStatus)
{
	m_iIsFileCompleted = iStatus;
}

LPCTSTR CListItem::GetJetInfo(int iIndex)
{
	return m_arJetsInfo[iIndex];
}


int CListItem::GetCountOfJets()
{
	return m_arJetsInfo.GetSize();
}

void CListItem::SetDirty(BOOL bDirty)
{
	m_bDirty = bDirty;
}

BOOL CListItem::IsDirty()
{
	return m_bDirty;
}
