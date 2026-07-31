#pragma once
class CGameItemUI : public DuiLib::COptionUI
{
public:
	CGameItemUI();
	~CGameItemUI(void);

public:
	void DoEvent(DuiLib::TEventUI& event);
	virtual void PaintStatusImage(IRenderContext& ctx);

public:
	void SetIcon(HICON hIcon);

private:
	HICON m_hIcon;
};

