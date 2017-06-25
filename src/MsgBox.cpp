/* Copyright (C) 2017 BARBOTIN Nicolas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "mgpcl/MsgBox.h"

#ifndef MGPCL_NO_GUI
#ifdef MGPCL_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static m::MsgBoxResult msgBox(const m::String &text, const m::String &title, m::MsgBoxButtons buttons, UINT type)
{
	switch(buttons) {
	case m::kMBB_Ok:
		type |= MB_OK;
		break;

	case m::kMBB_OkCancel:
		type |= MB_OKCANCEL;
		break;

	case m::kMBB_AbortRetryIgnore:
		type |= MB_ABORTRETRYIGNORE;
		break;

	case m::kMBB_YesNoCancel:
		type |= MB_YESNOCANCEL;
		break;

	case m::kMBB_YesNo:
		type |= MB_YESNO;
		break;

	case m::kMBB_RetryCancel:
		type |= MB_RETRYCANCEL;
		break;

	case m::kMBB_CancelTryAgainContinue:
		type |= MB_CANCELTRYCONTINUE;
		break;

	default:
		break;
	}

	int ret = MessageBox(nullptr, text.raw(), title.raw(), type);
	switch(ret) {
	case IDABORT:
		return m::kMBR_Abort;
		
	case IDCANCEL:
		return m::kMBR_Cancel;

	case IDCONTINUE:
		return m::kMBR_Continue;

	case IDIGNORE:
		return m::kMBR_Ignore;

	case IDYES:
		return m::kMBR_Yes;

	case IDNO:
		return m::kMBR_No;

	case IDTRYAGAIN:
		return m::kMBR_TryAgain;

	case IDOK:
		return m::kMBR_Ok;

	default:
		return m::kMBR_MsgBoxError;
	}
}

m::MsgBoxResult m::msgBox::info(const String &text, const String &title, MsgBoxButtons buttons)
{
	return ::msgBox(text, title, buttons, MB_ICONINFORMATION);
}

m::MsgBoxResult m::msgBox::warning(const String &text, const String &title, MsgBoxButtons buttons)
{
	return ::msgBox(text, title, buttons, MB_ICONWARNING);
}

m::MsgBoxResult m::msgBox::error(const String &text, const String &title, MsgBoxButtons buttons)
{
	return ::msgBox(text, title, buttons, MB_ICONEXCLAMATION);
}

m::MsgBoxResult m::msgBox::question(const String &text, const String &title, MsgBoxButtons buttons)
{
	return ::msgBox(text, title, buttons, MB_ICONQUESTION);
}

#else
#include "mgpcl/GUI.h"
#include <gtk/gtk.h>

static m::MsgBoxResult msgBox(const m::String &text, const m::String &title, m::MsgBoxButtons buttons, const char *type)
{
    if(!m::gui::hasBeenInitialized())
        m::gui::initialize();

    GtkWidget *dialog = gtk_dialog_new();
    g_object_ref_sink(dialog);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *icon = gtk_image_new_from_icon_name(type, GTK_ICON_SIZE_DIALOG);
    GtkWidget *label = gtk_label_new(text.raw());

    switch(buttons) {
        case m::kMBB_Ok:
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Ok", GTK_RESPONSE_ACCEPT);
            break;

        case m::kMBB_OkCancel:
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Ok", GTK_RESPONSE_ACCEPT);
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", GTK_RESPONSE_REJECT);
            break;

        case m::kMBB_AbortRetryIgnore:
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Abort", GTK_RESPONSE_CANCEL);
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Retry", GTK_RESPONSE_YES);
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Ignore", GTK_RESPONSE_NO);
            break;

        case m::kMBB_YesNoCancel:
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Yes", GTK_RESPONSE_YES);
            gtk_dialog_add_button(GTK_DIALOG(dialog), "No", GTK_RESPONSE_NO);
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", GTK_RESPONSE_CANCEL);
            break;

        case m::kMBB_YesNo:
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Yes", GTK_RESPONSE_ACCEPT);
            gtk_dialog_add_button(GTK_DIALOG(dialog), "No", GTK_RESPONSE_REJECT);
            break;

        case m::kMBB_RetryCancel:
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Retry", GTK_RESPONSE_ACCEPT);
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", GTK_RESPONSE_REJECT);
            break;

        case m::kMBB_CancelTryAgainContinue:
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", GTK_RESPONSE_CANCEL);
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Try Again", GTK_RESPONSE_YES);
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Continue", GTK_RESPONSE_NO);
            break;

        default:
            break;
    }

    gtk_window_set_title(GTK_WINDOW(dialog), title.raw());
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(label), 0.f);
    gtk_label_set_yalign(GTK_LABEL(label), 0.f);
    gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);
    gtk_widget_show_all(hbox);
    gtk_widget_set_size_request(dialog, 400, 50);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    gint ret = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_hide(dialog);
    gtk_widget_destroy(dialog);
    g_object_unref(dialog);

	//Run message loop for a while, to close this window
	while(gtk_events_pending())
		gtk_main_iteration();

    switch(buttons) {
        case m::kMBB_Ok:
            if(ret == GTK_RESPONSE_ACCEPT)
                return m::kMBR_Ok;

            break;

        case m::kMBB_OkCancel:
            if(ret == GTK_RESPONSE_ACCEPT)
                return m::kMBR_Ok;
            else if(ret == GTK_RESPONSE_REJECT)
                return m::kMBR_Cancel;

            break;

        case m::kMBB_AbortRetryIgnore:
            if(ret == GTK_RESPONSE_CANCEL)
                return m::kMBR_Abort;
            else if(ret == GTK_RESPONSE_YES)
                return m::kMBR_Retry;
            else if(ret == GTK_RESPONSE_NO)
                return m::kMBR_Ignore;

            break;

        case m::kMBB_YesNoCancel:
            if(ret == GTK_RESPONSE_YES)
                return m::kMBR_Yes;
            else if(ret == GTK_RESPONSE_NO)
                return m::kMBR_No;
            else if(ret == GTK_RESPONSE_CANCEL)
                return m::kMBR_Cancel;

            break;

        case m::kMBB_YesNo:
            if(ret == GTK_RESPONSE_ACCEPT)
                return m::kMBR_Yes;
            else if(ret == GTK_RESPONSE_REJECT)
                return m::kMBR_No;

            break;

        case m::kMBB_RetryCancel:
            if(ret == GTK_RESPONSE_ACCEPT)
                return m::kMBR_Retry;
            else if(ret == GTK_RESPONSE_REJECT)
                return m::kMBR_Cancel;

            break;

        case m::kMBB_CancelTryAgainContinue:
            if(ret == GTK_RESPONSE_CANCEL)
                return m::kMBR_Cancel;
            else if(ret == GTK_RESPONSE_YES)
                return m::kMBR_TryAgain;
            else if(ret == GTK_RESPONSE_NO)
                return m::kMBR_Continue;

            break;

        default:
            break;
    }

    return m::kMBR_MsgBoxError;
}

m::MsgBoxResult m::msgBox::info(const String &text, const String &title, MsgBoxButtons buttons)
{
	return ::msgBox(text, title, buttons, "dialog-information");
}

m::MsgBoxResult m::msgBox::warning(const String &text, const String &title, MsgBoxButtons buttons)
{
    return ::msgBox(text, title, buttons, "dialog-warning");
}

m::MsgBoxResult m::msgBox::error(const String &text, const String &title, MsgBoxButtons buttons)
{
    return ::msgBox(text, title, buttons, "dialog-error");
}

m::MsgBoxResult m::msgBox::question(const String &text, const String &title, MsgBoxButtons buttons)
{
    return ::msgBox(text, title, buttons, "dialog-question");
}

#endif
#endif
