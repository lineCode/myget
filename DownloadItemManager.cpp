// DownloadItemManager.cpp: implementation of the CDownloadItemManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "myget.h"
#include "DownloadItemManager.h"
#include "DownloadHttpJet.h"
#include "DownloadFtpJet.h"
//#include "MainScheduleThread.h"
#include "DownloadScheduler.h"

#include "AppRegs.h"

//tempory include, only for BLOCK_BYTE_SIZE.
//In the furtures, If we read the reg value from the registry, remove it!!!
#include "ProgressChart.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//IMPLEMENT_DYNCREATE(CDownloadItemManager, CWinThread)

//DEL BOOL CDownloadItemManager::InitInstance()
//DEL {
//DEL 	// TODO:  perform and per-thread initialization here
//DEL 	StartDownload();
//DEL 
//DEL 	return TRUE;
//DEL }






CDownloadItemManager::CDownloadItemManager(CDownloadScheduler *pDownloadScheduler, CListItem *pListItem) : 
		m_slockCanHandleMsg(&m_crtsectCanHandleMsg)
{
	//m_pMainScheduleThread = pMainScheduleThread;
	m_pDownloadScheduler = pDownloadScheduler;
	m_pListItem = pListItem;
	m_iSimJets = m_pListItem->GetSplitsCount();
	m_iResumeState = m_pListItem->IsSupportResumed();
	m_iDownloadFileSize = m_pListItem->GetFileSize();

	m_arJets.RemoveAll();

	m_bHasInitMaps = FALSE;

	m_pFile = NULL;

	m_bQuit = FALSE;

	if (m_iDownloadFileSize > 0)
	{
		m_bHasInitMaps = prv_InitMaps();
	}


}

CDownloadItemManager::~CDownloadItemManager()
{
	prv_ClearJob();

	CDownloadJet *pJet;
	for (int i = m_arJets.GetSize() - 1; i >= 0; i --)
	{
		pJet = m_arJets[i];

		if (pJet)
		{
			delete pJet;
		}
	}

	m_arJets.RemoveAll();

}


int CDownloadItemManager::StopDownload()
{
	prv_ClearJob();
	
	return 0;
}

int CDownloadItemManager::StartDownload()
{
	m_pListItem->SetFileStatus(DOWNLOADING);
	m_pListItem->ClearJetInfo();
	m_pDownloadScheduler->RefreshMainframe(m_pListItem);

	return prv_NewJet(TRUE);
}

int CDownloadItemManager::prv_GetThreadIndex(CDownloadJet *pJet)
{
	for (int i = 0; i < m_arJets.GetSize(); i ++)
	{
		if (m_arJets[i] == pJet)
		{
			return i;
		}
	}

	return -1;
}


void CDownloadItemManager::JetGeneralInfo(CDownloadJet *pJet, LPCTSTR lpszMsg)
{
	int iIndex = prv_GetThreadIndex(pJet);

	prv_RefreshItem(pJet, lpszMsg);
	TRACE("Thread <%d> sent header info \r\n", iIndex);

}

BOOL CDownloadItemManager::JetStartRecv(CDownloadJet *pJet, LPCTSTR lpszMsg)
{
	int iIndex = prv_GetThreadIndex(pJet);

	TRACE("Thread: %d Start to Recv\r\n", iIndex);

	prv_RefreshItem(pJet, lpszMsg);

	if (m_bQuit)
	{
		return FALSE;
	}

	m_pListItem->SetAttr(ATTR_EXCUTED, TRUE);

	if (pJet->IsSupportResume())
	{
		m_iResumeState = SUPPORTRESUME;
		m_pListItem->SetAttr(ATTR_SUPPORTRESUME, TRUE);
	}
	else if (iIndex > 0)
	{
		pJet->StopDownload();
	
		prv_MergeUndownloadedBrks();
		return FALSE;
	}
	
	//�����֧�ֶϵ������Ļ�����ô��ֻ�����ǵ�һ���̣߳����Ҵӿ�ʼ���أ�
	//���ܹ�����������������Ӧ����ResultFailed�д���

	/*
	if (pJet->GetRange()->iStart > 0 && !pJet->IsSupportResume())
	{
		m_iResumeState = UNSUPPORTRESUME;
		m_pListItem->SetAttr(ATTR_SUPPORTRESUME, FALSE);

		
		if (iIndex == 0)
		{
			prv_MergeUndownloadedBrks();
		}
		
		if (iIndex > 0)
		{
			//��ʱ�򣬺������߳�Ӧ�ò��������ء�
			pJet->StopDownload();
	
			return FALSE;
		}

		
	}
	*/

	//����ǵ�һ���̣߳������һЩ��ʼ������
	if (m_iDownloadFileSize == -1)
	{
		//��ͼ�ӵ�һ�����ص��߳��еõ����ص��ļ��ĳߴ磬������FTP����ʱ��Ӧ�ò��ñ�ķ�����
		m_iDownloadFileSize = m_pListItem->GetFileDownloadedSize() + pJet->GetContentLength();
		
		if (!m_bHasInitMaps)
		{
			if (!prv_InitMaps())
			{
				return FALSE;
			}
			
			m_bHasInitMaps = TRUE;

		}

		//�����ط�Χ���з���, ע��������ط�Χ����ʧ�ܣ�ʵ���ϸ��߳��Ѿ���ɾ������ص����ݽṹ�����¡�
		if (!prv_AssignDownloadRange(pJet))
		{
			pJet->StopDownload();
		}

	}

	//ֻ�е��ļ��ߴ���֪�����Ҳ���֪����֧�ֶϵ�������ʱ�򣬲ſ��Կ�ʼ�������̡߳�
	//ע�⣺����2���̷߳��ֲ�֧�ֶϵ�������ʱ�򣬻ᵼ��ResultFailed�ĵ��ã�������뵽���
	if (m_iResumeState != UNSUPPORTRESUME && m_iDownloadFileSize != -1)
	{
		prv_NewJet(FALSE);
	}

	return TRUE;
}

BOOL CDownloadItemManager::prv_InitDownloadJob()
{
	m_strFilePath= m_pListItem->GetDownloadedFolder();

	if (m_strFilePath.Right(1) != "\\")
	{
		m_strFilePath += "\\";
	}

	m_strFilePath += m_pListItem->GetRename();

	m_pFile = new CFile;

	m_pFile->Open(m_strFilePath + ".jc!", CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);

	m_pFile->SetLength(m_iDownloadFileSize);

	m_pListItem->SetFileSize(m_iDownloadFileSize);

	m_pFile->Flush();
	return TRUE;
}



void CDownloadItemManager::JetRecvOK(CDownloadJet *pJet, LPCTSTR lpszMsg)
{
	if (!m_bQuit)
	{
		m_pFile->Seek(pJet->GetCurPos() - pJet->GetCurDownloadedSize(), CFile::begin);
		m_pFile->Write(pJet->GetDownloadBuf(), pJet->GetCurDownloadedSize());

		prv_UpdateDownloadBrkMap(pJet);
		//m_pMainScheduleThread->IncLastDownloadDataCounter(pJet->GetCurDownloadedSize());
		m_pDownloadScheduler->IncLastDownloadDataCounter(pJet->GetCurDownloadedSize());

		prv_RefreshItem(pJet, lpszMsg);

	}
}

void CDownloadItemManager::JetCanceled(CDownloadJet *pJet, LPCTSTR lpszMsg)
{
	prv_RefreshItem(pJet, lpszMsg);
}
	
void CDownloadItemManager::JetResultFailed(CDownloadJet *pJet, LPCTSTR lpszMsg)
{
	prv_RefreshItem(pJet, lpszMsg);

	PBREAKPOSITION pBrkPos = pJet->GetRange();

	if (pBrkPos->iEnd == -1)
	{
		delete pBrkPos;
	}

	prv_ClearJob();

}

void CDownloadItemManager::JetFailed(CDownloadJet *pJet, LPCTSTR lpszMsg)
{
	prv_RefreshItem(pJet, lpszMsg);
}

void CDownloadItemManager::JetCompleted(CDownloadJet *pJet, LPCTSTR lpszMsg)
{
	TRACE("Thread: %d Completed Job\r\n", prv_GetThreadIndex(pJet));

	prv_RefreshItem(pJet, lpszMsg);

	prv_HandleThreadCompleted(pJet);

	if (m_bQuit)
	{
		return;
	}


	if (!prv_AssignDownloadRange(pJet))
	{
		prv_AttachJetInfo(pJet, lpszMsg);		
		pJet->StopDownload();
	}

	if (m_arUnDownloadedBrks.GetSize() == 0)
	{
		prv_ClearJob();
	}

}

BOOL CDownloadItemManager::prv_InitMaps()
{
	int i;

	if (m_iDownloadFileSize < 0)
	{
		return FALSE;
	}

	prv_InitDownloadJob();

	//���ȴ�ListItem�еõ��Ѿ����ص���Ƭ��Ϣ��
	m_parDownloadBrks = (CArray <PBREAKPOSITION, PBREAKPOSITION> *)m_pListItem->GetBrkPosArray();
	
	//�����δ���صļ�¼
	m_arUnDownloadedBrks.RemoveAll();
	
	//��ListItem�е��Ѿ����ص���Ƭ��¼��������δ�������Ƭ��¼
	PBREAKPOSITION pBrkPos;
	PBREAKPOSITION pNewBrkPos;

	int iLastDowndPos = m_iDownloadFileSize;

	for (i = m_parDownloadBrks->GetSize() - 1; i >= 0; i --)
	{
		pBrkPos = m_parDownloadBrks->GetAt(i);
		
		if (pBrkPos->iEnd < iLastDowndPos)
		{
			pNewBrkPos = new BREAKPOSITION;

			pNewBrkPos->iEnd = iLastDowndPos;
			pNewBrkPos->iStart = pBrkPos->iEnd;

			m_arUnDownloadedBrks.InsertAt(0, pNewBrkPos);

			iLastDowndPos = pBrkPos->iStart;
		}
	}

	if (m_parDownloadBrks->GetSize() == 0)
	{
		//�����һ�������ص��ļ�����û�������صļ�¼����Ӧ�ð�����ļ�������Ϊһ���Ѿ����صļ�¼��
		pNewBrkPos = new BREAKPOSITION;
		pNewBrkPos->iStart	= 0;
		pNewBrkPos->iEnd	= iLastDowndPos;

		m_arUnDownloadedBrks.InsertAt(0, pNewBrkPos);
	}

	//�����֪֧�ֶϵ���������ô������Ҫ����δ���صļ�¼���зָ�
	CAppRegs *pAppRegs = ((CMyGetApp *)AfxGetApp())->GetAppRegs();

	//int iMinAssignSize = pAppRegs->GetVal(REG_GENERAL_BLOCKSIZE, &iMinAssignSize);
	//	RegProcess.GetDWORD(REG_GENERAL_BLOCK_SIZE);
	int iMinAssignSize;
	pAppRegs->GetVal(REG_GENERAL_BLOCKSIZE, (DWORD *)&iMinAssignSize);
	int iMaxBrkSize = -1;
	int iMaxBrkIndex = 0;

	
	//ʵ���ϣ��ڸտ�ʼ�����ǿ��ܻ���֪��һ���µ������ļ��Ƿ����������
	//�����Ƚ��зָ�Ժ����ȷ��֪������֧�������������ٽ��кϲ���
	/*
	//2002-03-29
	//�����֪��һ���µ������ļ��Ƿ�����������;ܾ��ָ
	if (m_iResumeState == SUPPORTRESUME)*/
	if (m_iResumeState != UNSUPPORTRESUME)
	{
		while(m_iSimJets > m_arUnDownloadedBrks.GetSize())
		{
			//�ҵ�����һ����Ƭ
			for (i = 0; i < m_arUnDownloadedBrks.GetSize(); i ++)
			{
				pBrkPos = m_arUnDownloadedBrks[i];
				
				if (iMaxBrkSize < (pBrkPos->iEnd - pBrkPos->iStart))
				{
					iMaxBrkSize = pBrkPos->iEnd - pBrkPos->iStart;
					iMaxBrkIndex = i;
				}
			}

			if (iMaxBrkSize < iMinAssignSize * 2)
			{
				break;
			}

			pNewBrkPos = new BREAKPOSITION;
			pBrkPos = m_arUnDownloadedBrks[iMaxBrkIndex];
			pNewBrkPos->iEnd = pBrkPos->iEnd;
			pBrkPos->iEnd = ((pBrkPos->iStart + pBrkPos->iEnd) / 2 & (0xFFFFFFFF - BLOCK_BYTE_SIZE + 1));
			pNewBrkPos->iStart = pBrkPos->iEnd;

			m_arUnDownloadedBrks.InsertAt(iMaxBrkIndex + 1, pNewBrkPos);
		}
	}

	CString strDebug;

	TRACE("\r\n\r\n");
	for (int j = 0; j < m_arUnDownloadedBrks.GetSize(); j ++)
	{
		strDebug.Format("Download Range <%d>: From: %d --- To: %d \r\n", 
					j, m_arUnDownloadedBrks[j]->iStart,
					m_arUnDownloadedBrks[j]->iEnd);
		TRACE(strDebug);
	}

	TRACE("\r\n\r\n");

	return TRUE;
}

//�߳������µ���������
BOOL CDownloadItemManager::prv_AssignDownloadRange(CDownloadJet *pJet)
{
	PBREAKPOSITION pOldBrkPos = pJet->GetRange();

	if (pOldBrkPos && pOldBrkPos->iEnd == -1)
	{
		//It is the temporty range, should be deleted.
		delete pOldBrkPos;
	}


	//�����û�����ص���Ƭ������ͼ���䣬���򷵻�FALSE����ζ�Ÿ��߳��Ѿ�������
	if (m_arUnDownloadedBrks.GetSize() > 0)
	{
		//������ԣ������û�б��߳�ʹ�õ���Ƭ���ҵ�����һƬ������֮��
		//�����ҵ�����һƬ����ͼ�з֣��ɹ����򽫺�һ�����֮��
		PBREAKPOSITION pBrkPos;	

		int iFirstUnAssignedIndex = -1; 
		int iMaxAssignedIndex   = -1;

		//int iMaxUnAssignedSize	= -1;
		int iMaxAssignedSize	= -1;

		for (int i = 0; i < m_arUnDownloadedBrks.GetSize(); i ++)
		{	
			pBrkPos = m_arUnDownloadedBrks[i];

			if (prv_HasAssigned(pBrkPos))
			{
				if (iMaxAssignedSize < pBrkPos->iEnd - pBrkPos->iStart)
				{
					iMaxAssignedSize	= pBrkPos->iEnd - pBrkPos->iStart;
					iMaxAssignedIndex	= i;
				}
			}
			else
			{
				iFirstUnAssignedIndex	= i;

				break;
			}
		}

		if (iFirstUnAssignedIndex >= 0)
		{
			//�������δ�������Ƭ�����������̡߳�
			pBrkPos = m_arUnDownloadedBrks[iFirstUnAssignedIndex];

			pJet->SetRange(pBrkPos);

			CString strDebug;
			strDebug.Format("Assign Range to Thread: <%d>, Range from: %d --- TO: %d \r\n",
					prv_GetThreadIndex(pJet), pBrkPos->iStart, pBrkPos->iEnd);

			TRACE(strDebug);
		
			return TRUE;
		}
		else if (iMaxAssignedIndex >= 0)
		{
			//return FALSE;
			CAppRegs *pAppRegs = ((CMyGetApp *)AfxGetApp())->GetAppRegs();

			//int iMinAssignSize; = RegProcess.GetDWORD(REG_GENERAL_BLOCK_SIZE);
			int iMinAssignSize;
			pAppRegs->GetVal(REG_GENERAL_BLOCKSIZE, (DWORD *)&iMinAssignSize);

			//���û�п������Ƭ������ͼ�и�����һƬ��
			if (iMaxAssignedSize >= iMinAssignSize * 2)
			{
				PBREAKPOSITION pNewBrkPos = new BREAKPOSITION;
				pBrkPos = m_arUnDownloadedBrks[iMaxAssignedIndex];
				pNewBrkPos->iEnd = pBrkPos->iEnd;
				pBrkPos->iEnd = (pBrkPos->iStart + pBrkPos->iEnd) / 2;
				pNewBrkPos->iStart = pBrkPos->iEnd;

				m_arUnDownloadedBrks.InsertAt(iMaxAssignedIndex + 1, pNewBrkPos);

				pJet->SetRange(pNewBrkPos);

				CString strDebug;
				strDebug.Format("Assign Range to Thread: <%d>, Range from: %d --- TO: %d \r\n",
					prv_GetThreadIndex(pJet), pBrkPos->iStart, pBrkPos->iEnd);
	
				TRACE(strDebug);

				return TRUE;
			}
		}

	}
	

	return FALSE;
}


BOOL CDownloadItemManager::prv_HasAssigned(PBREAKPOSITION pBrkPos)
{
	for (int i = 0; i < m_arJets.GetSize(); i ++)
	{
		if (pBrkPos == m_arJets[i]->GetRange())
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CDownloadItemManager::prv_MergeUndownloadedBrks()
{
	//�����Ѿ�֪��Ҫ���ص��ļ���֧����������Ҫ�����еĿ��Ժϲ�����Ƭ�ϳ�һ�顣
	//�����޸�Ϊ����Ҫ��Ե�һ���߳�Ϊ���Ľ��кϲ�����Ϊ�п��ܳ��ֵ�һ���߳��Ѿ������ص�3������������
	//�����ص�����ǣ������ʱ��һ���߳��Ѿ�ֹͣ�Ļ�����Ҫ����������һ���̡߳����Ժ��ٿ��ǣ�
	int iIndex = 0;

	PBREAKPOSITION pCurBrkPos;
	PBREAKPOSITION pNextBrkPos;

	CDownloadJet *pJet = m_arJets[0];
	PBREAKPOSITION pJet0Pos = pJet->GetRange();

	while (iIndex < m_arUnDownloadedBrks.GetSize() - 1)
	{
		pCurBrkPos	= m_arUnDownloadedBrks[iIndex];
		pNextBrkPos = m_arUnDownloadedBrks[iIndex + 1];

		if (pCurBrkPos->iEnd == pNextBrkPos->iStart && pJet0Pos != pNextBrkPos)
		{
			pCurBrkPos->iEnd = pNextBrkPos->iEnd;

			m_arUnDownloadedBrks.RemoveAt(iIndex + 1);

			delete pNextBrkPos;
		}
		else
		{
			iIndex ++;
		}
	}
}

BOOL CDownloadItemManager::prv_NewJet(BOOL bIsFirst)
{
	CDownloadJet *pJet;

	if (m_arJets.GetSize() >= m_iSimJets)
	{
		return FALSE;
	}

	if (!bIsFirst && (m_iDownloadFileSize == -1 || m_iResumeState == UNSUPPORTRESUME))
	{
		return FALSE;
	}

	//��һ���жϣ���������http����ftp jet.
	CString strTemp = m_pListItem->GetURL();
	strTemp = strTemp.Left(6);
	strTemp.MakeLower();
	if( strTemp.Compare("ftp://") == 0 )
	{
		pJet = new CDownloadFtpJet(this);
	}
	else
	{
		pJet = new CDownloadHttpJet(this);
	}

//	pJet->SetNotifyWnd(m_pDownloadScheduler);

	pJet->SetURL(m_pListItem->GetURL());

	PBREAKPOSITION pBrkPos;
	if (m_iDownloadFileSize == -1)
	{
		pBrkPos = new BREAKPOSITION;
		pBrkPos->iStart = m_pListItem->GetFileDownloadedSize();
		pBrkPos->iEnd = -1;

		pJet->SetRange(pBrkPos);
	}
	else
	{
		if (!prv_AssignDownloadRange(pJet))
		{
			delete pJet;

			return FALSE;
		}
	}

	m_arJets.Add(pJet);

	pJet->SetRetry(RETRY_TYPE_ALWAYS, 3000);
	
	pJet->StartDownload();

	CString strNewThread;
	strNewThread.Format("New Thread <%d> ---- \r\n", prv_GetThreadIndex(pJet));

	TRACE(strNewThread);

	return TRUE;
}

void CDownloadItemManager::prv_ClearJob()
{
	int i;
	CDownloadJet *pJet;

	//Ask all the threads to stop.

	for (i = m_arJets.GetSize() - 1; i >= 0; i --)
	{
		pJet = m_arJets[i];

		pJet->StopDownload();
		
		//delete pJet;
	}


	//m_arJets.RemoveAll();

	

	if (m_pFile)
	{
		m_pFile->Flush();
		m_pFile->Close();

		if (m_pListItem->GetFileDownloadedSize() == m_pListItem->GetFileSize())
		{
			DeleteFile(m_strFilePath);

			m_pFile->Rename(m_strFilePath + ".jc!", m_strFilePath);
			m_pListItem->SetFileStatus(COMPLETED);

			m_pDownloadScheduler->RefreshMainframe(m_pListItem);

			//If the download item is completed, it should be moved to the specified category.
			m_pDownloadScheduler->MoveCompletedItem(m_pListItem);
		}

		delete m_pFile;
		m_pFile = NULL;
	}

	

	PBREAKPOSITION pBrkPos;
	for (i = 0 ; i < m_arUnDownloadedBrks.GetSize(); i ++)
	{	
		pBrkPos = m_arUnDownloadedBrks[i];

		delete pBrkPos;
		pBrkPos = NULL;
	}

	m_arUnDownloadedBrks.RemoveAll();
	
	//m_pMainScheduleThread->PostMessage(DOWNLOAD_JOB_END, (DWORD)this, 0);
	m_pDownloadScheduler->PostMessage(DOWNLOAD_JOB_END, (DWORD)this, 0);

}

void CDownloadItemManager::prv_UpdateDownloadBrkMap(CDownloadJet *pJet)
{
	//���ݵ�ǰ���̵߳�ɾ����Χ�������Ѿ����صļ�¼��
	//ע�⣺��ǰ���̵߳����ط�Χ�Ѿ��Զ����£�������߳����ط�ΧΪ�գ�����ɾ����
	//Ҫ��CompleteJob�¼���ɾ����

	PBREAKPOSITION pBrkPos = pJet->GetRange();

	int iStartPos = pBrkPos->iStart - pJet->GetCurDownloadedSize();

	//����Ҫ���Ѿ����صļ�¼�в���ͬ������صĿ�ʼ��ַ��ͬ�ļ�¼��

	int iIndex = -1;
	BOOL bMatched = FALSE;

	for (int i = 0; i < m_parDownloadBrks->GetSize(); i ++)
	{
		if (m_parDownloadBrks->GetAt(i)->iEnd == iStartPos)
		{
			bMatched = TRUE;
			iIndex = i;

			break;
		}
		else if (m_parDownloadBrks->GetAt(i)->iEnd < iStartPos)
		{
			iIndex = i;
		}
	}

	if (bMatched)
	{
		m_parDownloadBrks->GetAt(iIndex)->iEnd = pBrkPos->iStart;
	}
	else
	{
		PBREAKPOSITION pNewBrkPos = new BREAKPOSITION;
	
		pNewBrkPos->iStart = iStartPos;
		pNewBrkPos->iEnd = pBrkPos->iStart;

		m_parDownloadBrks->InsertAt(iIndex + 1, pNewBrkPos);
	}
}

void CDownloadItemManager::prv_HandleThreadCompleted(CDownloadJet *pJet)
{
	PBREAKPOSITION pBrkPos = pJet->GetRange();

	for (int i = 0; i < m_arUnDownloadedBrks.GetSize(); i ++)
	{
		if (pBrkPos == m_arUnDownloadedBrks[i])
		{
			m_arUnDownloadedBrks.RemoveAt(i);

			break;
		}
	}

	delete pBrkPos;
	pJet->SetRange(NULL);


}


void CDownloadItemManager::prv_AttachJetInfo(CDownloadJet *pJet, LPCTSTR lpszMsg)
{
	int iIndex = prv_GetThreadIndex(pJet);

	if (iIndex > -1)
	{
		//GlobalLock((void *)lpszMsg);
		m_pListItem->AddJetInfo(iIndex, lpszMsg);
		//GlobalUnlock((void *)lpszMsg);
	}

	//GlobalFree((void *)lpszMsg);
	
}

CListItem * CDownloadItemManager::GetListItem()
{
	return m_pListItem;
}


/*
BOOL CDownloadItemManager::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

	if (m_bQuit)
	{
		if (pMsg->message == WM_QUIT)
		{
			return CWinThread::PreTranslateMessage(pMsg);
		}
		else
		{
			return TRUE;
		}
	}

//	m_slockCanHandleMsg.Lock();


	CDownloadJet	*pJet = (CDownloadJet *)pMsg->wParam;
	LPCTSTR lpszMsg = (LPCTSTR)pMsg->lParam;

	switch(pMsg->message)
	{
	case WM_USER_DOWNLOAD_THREAD_GENERAL_INFO:
		prv_OnGeneralInfo(pJet, lpszMsg);
		break;
	
	case WM_USER_DOWNLOAD_THREAD_START_RECV:
		prv_OnStartRecv(pJet, lpszMsg);
		break;

	case WM_USER_DOWNLOAD_THREAD_RECV_OK:
		prv_OnRecvOK(pJet, lpszMsg);
		break;
		
	case WM_USER_DOWNLOAD_THREAD_COMPLETED:
		prv_OnJobCompleted(pJet, lpszMsg);
		break;

	case WM_USER_DOWNLOAD_THREAD_FAILED:
		prv_OnThreadFailed(pJet, lpszMsg);
		break;

	case WM_USER_DOWNLOAD_THREAD_CANCELED:
		prv_OnThreadCanceled(pJet, lpszMsg);
		break;

	case WM_USER_DOWNLOAD_THREAD_ERROR:
		prv_OnThreadResultFailed(pJet, lpszMsg);
		break;

	case DOWNLOAD_JOB_END:
		m_bQuit = TRUE;
		prv_ClearJob();
		//m_pMainScheduleThread->PostThreadMessage(DOWNLOAD_JOB_END, (DWORD)this, 0);		
		break;

	default:
//		m_slockCanHandleMsg.Unlock();
		return CWinThread::PreTranslateMessage(pMsg);

	}

	//m_pMainScheduleThread->RefreshMainFrame(m_pListItem);
	m_pDownloadScheduler->RefreshMainFrame(m_pListItem);

//	m_slockCanHandleMsg.Unlock();
		
	return TRUE;		
}
*/


BOOL CDownloadItemManager::AddDownloadEntry(CDownloadJet *pJet, SOCKET socket)
{
	return m_pDownloadScheduler->AddDownloadEntry(this, pJet, socket);
}

void CDownloadItemManager::RegisterTimeout(CDownloadJet *pJet, DWORD dwTimeout)
{
	m_pDownloadScheduler->RegisterTimeout(pJet, dwTimeout);
}

void CDownloadItemManager::UnregisterTimeout(CDownloadJet *pJet)
{
	m_pDownloadScheduler->UnregisterTimeout(pJet);
}



void CDownloadItemManager::RemoveDownloadEntry(CDownloadJet *pJet)
{
	m_pDownloadScheduler->RemoveDownloadEntry(pJet, this);
}

void CDownloadItemManager::prv_RefreshItem(CDownloadJet *pJet, LPCTSTR lpszMsg)
{
	if (lpszMsg)
	{
		prv_AttachJetInfo(pJet, lpszMsg);
	}

	m_pDownloadScheduler->RefreshMainframe(m_pListItem, 0);
	//m_pDownloadScheduler->PostMessage(WM_DOWNLOAD_ITEM_MSG, (WPARAM)m_pListItem, (LPARAM)0);
}
