# Copyright (C) 2016- Bjarne Yang
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
# 感谢科大讯飞提供免费语音合成引擎支持 (http://www.iflytek.com/)
#
#

'''
增加上下文菜单来转换中文文本到语音
'''

from gettext import gettext as _
from gi.repository import Gtk, Gedit, Gio, GObject, GtkSource
import re
import sys
import os
import subprocess
import string
import ctypes
import os.path
import simpleaudio



class ChineseToSpeechPlugin(GObject.Object, Gedit.WindowActivatable):
	__gtype_name__ = "ChineseToSpeechPlugin"
	window = GObject.property(type=Gedit.Window)

	def __init__(self):
		GObject.Object.__init__(self)
		self.text = ""
		self.window = None
		self.encoding = GtkSource.Encoding.get_from_charset("UTF-8")

	def do_activate(self):
		handler_ids = []
		for signal in ('tab-added', 'tab-removed'):
			method = getattr(self, 'on_window_' + signal.replace('-', '_'))
			handler_ids.append(self.window.connect(signal, method))
		self.window.ChineseToSpeechPluginID = handler_ids
		for view in self.window.get_views():
			self.connect_view(view)

	def do_deactivate(self):
		widgets = [self.window] + self.window.get_views()
		for widget in widgets:
			handler_ids = widget.ChineseToSpeechPluginID
			if not handler_ids is None:
				for handler_id in handler_ids:
					widget.disconnect(handler_id)
			widget.ChineseToSpeechPluginID = None
		self.window = None

	def connect_view(self, view):
		handler_id = view.connect('populate-popup', self.on_view_populate_popup)
		view.ChineseToSpeechPluginID = [handler_id]


	def update_ui(self, window):
		pass

	def on_window_tab_added(self, window, tab):
		self.connect_view(tab.get_view())

	def on_window_tab_removed(self, window, tab):
		pass

	def on_view_populate_popup(self, view, menu):
		doc = view.get_buffer()
		text=''
		if doc.get_selection_bounds():
			start, end = doc.get_selection_bounds()
			text = doc.get_text(start, end, False)

		if len(text) == 0:
			return True

		text = self.is_chinese_text(text)
		#检查是否为中文
		if not text:
			return True

		text_speech_item = Gtk.ImageMenuItem(_("To Speech"))
		text_speech_item.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_JUMP_TO,Gtk.IconSize.MENU))
		text_speech_item.connect('activate', self.on_chinese_to_speech_activate, text);
		text_speech_item.show();

		separator = Gtk.SeparatorMenuItem()
		separator.show();
		menu.prepend(separator)
		menu.prepend(text_speech_item)
		return True

	def on_chinese_to_speech_activate(self, menu_item, text):
		self.text_to_speech(text)
		return True

	def is_chinese_text(self, text):
		if len(text) > 0:
			return text
		return False

	def text_to_speech(self,text):
		cts=ctypes.cdll.LoadLibrary('/usr/local/lib/libcts.so') #加载动态库
		ret=cts.login() #登录科大讯飞
		if ret != 0 :
			print("cts.login error ",ret)
			return False

		path='/tmp/text_to_speech.wav'
		c_text=ctypes.c_char_p(text.encode("utf-8"))
		c_path=ctypes.c_char_p(path.encode("ascii"))
	
		ret=cts.speech(c_text,c_path)

		if ret != 0 :
			print("cts.speech error ",ret)
			return False

		play_obj = simpleaudio.WaveObject.from_wave_file(path).play()
		play_obj.wait_done()

		cts.logout() #退出科大讯飞

