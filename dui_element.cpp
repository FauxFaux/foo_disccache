#include "stdafx.h"
#include "dui_element.h"
#include "config.h"

dui_element::dui_element(ui_element_config::ptr config,ui_element_instance_callback_ptr p_callback) : 
	m_callback(p_callback), m_config(config)   {
	TRACK_CALL_TEXT("dui_element::dui_element");
#ifdef _DEBUG
	console::formatter() << "Constructing ui_element.";
#endif
	
	
	// load current settings to m_settings
	ui_element_config_parser parser(m_config);
	parser >> m_settings;		

	m_listview.SetCallback(p_callback);	
}

void dui_element::edit_mode_context_menu_build(const POINT & p_point,bool p_fromkeyboard,HMENU p_menu,unsigned p_id_base) { 
	TRACK_CALL_TEXT("dui_element::edit_mode_context_menu_build");
	CMenuHandle menu(p_menu);
	m_listview.BuildContextMenu(p_point, menu, p_id_base);	
}

void dui_element::edit_mode_context_menu_command(const POINT & p_point,bool p_fromkeyboard,unsigned p_id,unsigned p_id_base) {
	TRACK_CALL_TEXT("dui_element::edit_mode_context_menu_command");
	m_listview.CommandContextMenu(p_point, p_id, p_id_base);
}

bool dui_element::edit_mode_context_menu_get_focus_point(POINT & p_point) {	
	TRACK_CALL_TEXT("dui_element::edit_mode_context_menu_get_focus_point");
	CPoint pt(p_point);
	m_listview.EditModeContextMenuGetFocusPoint(pt);	
	p_point = pt;
	return true;
}

void dui_element::save_configuration() {
	TRACK_CALL_TEXT("dui_element::save_configuration");
	ui_element_config_builder config_builder;
	config_builder << m_settings;
	m_config = config_builder.finish(g_get_guid());
	
}

void dui_element::initialize_window(HWND parent) {	
	TRACK_CALL_TEXT("dui_element::initialize_window");
	Create(parent);
}

HWND dui_element::get_wnd() {	
	return *this;
}

void dui_element::RefreshVisuals()  {
	TRACK_CALL_TEXT("dui_element::RefreshVisuals");
	t_ui_font listfont;
	t_ui_color backgroundcolor;
	//t_ui_color highlightcolor;
	t_ui_color selectioncolor;
	t_ui_color textcolor;
	// Hardcoded color value http://www.hydrogenaudio.org/forums/index.php?showtopic=79981&pid=698341&mode=threaded&start=#entry698341
	t_ui_color insertmarkercolor = RGB(127,127,127);
	t_ui_color selectionrectanglecolor = RGB(127,127,127);
	t_ui_color selectedtextcolor;

	// Set font to listbox control
	listfont = m_callback->query_font_ex(ui_font_lists); 
	m_listview.SetFont(listfont);
	
	// Query colors and feed them forward to the listbox
	backgroundcolor = m_callback->query_std_color(ui_color_background);
	//highlightcolor = m_callback->query_std_color(ui_color_highlight);
	selectioncolor = m_callback->query_std_color(ui_color_selection);
	textcolor = m_callback->query_std_color(ui_color_text);
	selectedtextcolor = default_ui_hacks::DetermineSelectedTextColor(selectioncolor);
	m_listview.SetColors(backgroundcolor, selectioncolor, textcolor, insertmarkercolor, selectionrectanglecolor, selectedtextcolor);

	Invalidate();
}

void dui_element::notify(const GUID & p_what, t_size p_param1, const void * p_param2, t_size p_param2size) {
	TRACK_CALL_TEXT("dui_element::notify");
	if (p_what == ui_element_notify_colors_changed || p_what == ui_element_notify_font_changed) {
		// we use global colors and fonts - trigger a repaint whenever these change.		
		RefreshVisuals();
	}
}

void dui_element::OnFinalMessage(HWND hWnd){
	TRACK_CALL_TEXT("dui_element::OnFinalMessage");
	ui_element_base::OnFinalMessage(hWnd);

	// Let the base class(es) handle this message...
	SetMsgHandled(FALSE);
}


static service_factory_single_t<ui_element_myimpl> g_ui_element_myimpl_factory;


