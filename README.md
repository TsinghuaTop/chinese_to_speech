# chinese_to_speech
Adds context menu item to transfer chinese to speech and play it.

本插件使用科大讯飞中文语音合成引擎(http://www.iflytek.com/),感谢科大讯飞股份有限公司。

依赖：
1.python3 (新版ubuntu gedit已支持)
2.simpleaudio 安装方法 http://simpleaudio.readthedocs.io/en/latest/installation.html



安装方法
1.在科大讯飞网站申请帐号密码并填写在chinese2speech.cpp的login函数处。
2.make 
3.sudo cp libcts.so /usr/local/lib
  sudo cp lib/libmsc.so /usr/local/lib
4.cp chinese-to-speech.py ~/.local/share/gedit/plugins
  cp chinese-to-speech.plugin ~/.local/share/gedit/plugins
5.打开gedit文本编辑器
6.菜单->edit->Preferences->plugins->勾选 <Chinese To Speech> 以开启功能
7.打开中文文本->选中句子->右键 此时你会看到<To Speech>的菜单 
8.点击<To Speech>菜单 即可发声