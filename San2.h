#pragma once


// CSan2 �_�C�A���O

class CSan2 : public CDialog
{
	DECLARE_DYNAMIC(CSan2)

public:
	CSan2(CWnd* pParent = NULL);   // �W���R���X�g���N�^
	virtual ~CSan2();
	CString ret;
	int ret2;
	CListBox	m_list;
	CString Gett(int a);

// �_�C�A���O �f�[�^
	enum { IDD = IDD_SENTAKU24 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	afx_msg void OnDblclkList1();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};