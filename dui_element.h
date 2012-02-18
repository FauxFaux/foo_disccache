#pragma once

#include "stdafx.h"
#include "ui_element_base.h"

class dui_element : public ATL::CDialogImpl<dui_element>, public ui_element_instance, 
	private ui_element_base { 

public:
	//dialog resource ID
	enum {IDD = IDD_FORMVIEW};

	// when window dies
	virtual void OnFinalMessage(HWND /*hWnd*/);
	

	// DUI element stuff
	virtual HWND get_wnd();
	void initialize_window(HWND parent) ;	
	dui_element(ui_element_config::ptr, ui_element_instance_callback_ptr p_callback);
	void set_configuration(ui_element_config::ptr config) {m_config = config;}
	ui_element_config::ptr get_configuration() {return m_config;}
	static GUID g_get_guid() {		
		// {40D7E086-1033-43BC-B7BB-3D3848B2B346}
		static const GUID guid_myelem = 
		{ 0x40d7e086, 0x1033, 0x43bc, { 0xb7, 0xbb, 0x3d, 0x38, 0x48, 0xb2, 0xb3, 0x46 } };
		return guid_myelem;
	}
	static GUID g_get_subclass() {return ui_element_subclass_utility;}
	static void g_get_name(pfc::string_base & out) {out = COMPONENTNAME;}
	static ui_element_config::ptr g_get_default_configuration() {return ui_element_config::g_create_empty(g_get_guid());}
	static const char * g_get_description() {return COMPONENTNAME;}
	void notify(const GUID & p_what, t_size p_param1, const void * p_param2, t_size p_param2size);

	virtual bool edit_mode_context_menu_test(const POINT & p_point,bool p_fromkeyboard) {
		return true;
	}
	virtual void edit_mode_context_menu_build(const POINT & p_point,bool p_fromkeyboard,HMENU p_menu,unsigned p_id_base);
	virtual void edit_mode_context_menu_command(const POINT & p_point,bool p_fromkeyboard,unsigned p_id,unsigned p_id_base);
	virtual bool edit_mode_context_menu_get_focus_point(POINT & p_point);
	virtual bool edit_mode_context_menu_get_description(unsigned p_id,unsigned p_id_base,pfc::string_base & p_out) {return false;}

	
	// UI element configuration
	virtual void save_configuration();	

	virtual bool is_dui();

	//WTL message map
	BEGIN_MSG_MAP_EX(dui_element)
		MSG_WM_INITDIALOG(OnInitDialog)	// Init code
		MSG_WM_SIZE(OnSize) // Handle resize	
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		CHAIN_MSG_MAP_MEMBER(m_listview)
	END_MSG_MAP()


private:		
	void RefreshVisuals() ;	
	ui_element_config::ptr m_config;	

protected:
	// this must be declared as protected for ui_element_impl_withpopup<> to work.
	const ui_element_instance_callback_ptr m_callback;	
};


// ui_element_impl_withpopup autogenerates standalone version of our component and proper menu commands. Use ui_element_impl instead if you don't want that.
class ui_element_myimpl : public ui_element_impl_withpopup<dui_element> {};
