#pragma once


// CArc �_�C�A���O

class CArc : public CDialog
{
	DECLARE_DYNAMIC(CArc)

public:
	CArc(CWnd* pParent = NULL);   // �W���R���X�g���N�^
	virtual ~CArc();
	CString ret;
	int ret2;
	CListBox	m_list;
	CString Gett(int a);

// �_�C�A���O �f�[�^
	enum { IDD = IDD_SENTAKU22 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	afx_msg void OnDblclkList1();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};