/*
    This file is part of "GapLit Led Strip".

    "GapLit Led Strip" is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    "GapLit Led Strip" is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with "GapLit Led Strip".  If not, see <http://www.gnu.org/licenses/>.
*/

/*
  Website template in english

  All visible HTML should be contained in this file.
*/
#ifndef __LocalWebsite_lang_H_
#define __LocalWebsite_lang_H_

#define HTML_HEAD_BEGIN \
  "<head>"
#define HTML_HEAD_END \
  "</head>"

#define HTML_COMMON_CSS \
  "div,fieldset,input,select{padding:5px;font-size:1em;}"\
  "input{width:100%;box-sizing:border-box;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;}"\
  "select{width:100%;}"\
  "textarea{resize:none;width:98%;height:318px;padding:5px;overflow:auto;}"\
  "body{text-align:center;font-family:Arial,Helvetica,Sans-Serif;}"\
  "td{padding:0px;}"\
  "button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;-webkit-transition-duration:0.4s;transition-duration:0.4s;cursor:pointer;}"\
  "button:hover{background-color:#006cba;}"\
  "a{text-decoration:none;}"\
  ".p{float:left;text-align:left;}"\
  ".q{float:right;text-align:right;}\n"\
  ".lsS{width:45px;text-align:right;}.lsM{width:100px;text-align:right;}.lsL{width:150px;text-align:right;}\n"\
  ".ls{vertical-align:top;display:inline-block;text-align:center;width:120px;}\n"\
  ".lsimgON{width:100px;height:100px;} .lstxtON{display:inline-block;}\n"\
  ".lsimgOFF{width:100px;height:100px;} .lstxtOFF{display:inline-block;}\n"\
  ".inputfile { width: 0.1px; height: 0.1px; opacity: 0; overflow: hidden; position: absolute; z-index: -1; }\n"\
  ".inputfile + label { line-height:2.4rem;font-size:1.2rem;width:100%;-webkit-transition-duration:0.4s;transition-duration:0.4s;color: white; background-color: black; display: inline-block; cursor: pointer; }\n"\
  ".inputfile:focus + label, .inputfile + label:hover { color: black; background-color:#ececec;}\n"\
  ".inputfile:focus + label { outline: 1px dotted #000; outline: -webkit-focus-ring-color auto -5px;}\n"\
  ".inputfile + label * { pointer-events: none; }"\
  "input[type=radio] { display: inline; }"\
  "#snv div{width:100%;box-sizing:border-box;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;}"\
  ".snvName{width:40%;text-align:right;display:inline-block;}"\
  ".snvValue{width:60%;text-align:left;display:inline-block;box-sizing:border-box;}"\
  ".secTitle{text-align:left;font-size:1em;padding:5px}"\
  "#version{-webkit-transition-duration:0.4s;transition-duration:0.4s;}"\
  ""
#define HTML_COMMON1_JS \
  "function toggleLS(index){ var xhr = new XMLHttpRequest();xhr.onload = function(){ try {eval(xhr.responseText);}catch(error) {console.log(error);}}; xhr.open ('post', '/toggle', true); var d = new FormData(); d.append('light', index);  xhr.send(d); return false;}\n" \
  "function addLS(index) {var element = document.createElement('span');element.name='LS-'+index;element.id='LS-'+index;element.className='ls';element.onclick=function(){toggleLS(index);};var switches=document.getElementById('lt');switches.appendChild(element);}\n" \
  "function updateLS(index, label, value) {var ls = document.getElementById('LS-'+index);if (ls===null) {addLS(index);ls = document.getElementById('LS-' + index);}; var lsv = document.getElementById('LS-state-' + index); if (lsv == null || value != lsv.value) if (value == 'On') { ls.innerHTML=\"<input type='hidden' id='LS-state-\" + index + \"' value='On'><img src='/res?file=light-on.svg' class='lsimgON' alt='light on'><span class='lstxtON'>\" + label + \"</span>\";} else { ls.innerHTML=\"<input type='hidden' id='LS-state-\" + index + \"' value='Off'><img src='/res?file=light-off.svg' class='lsimgOFF' alt='light off'><span class='lstxtOFF'>\" + label + \"</span>\";}}\n" \
  "function reboot(){var xhr = new XMLHttpRequest(); xhr.onload = function(){ eval(xhr.responseText); }; xhr.open ('post', '/reboot', true); var d = new FormData(); d.append('reboot', 'true'); xhr.send(d); return false;}\n" \
  "function updateVersion(s) { document.getElementById('version').innerHTML = s; }\n" \
  "function getVersion() { var xhr = new XMLHttpRequest(); xhr.onload = function(){ try { updateVersion(xhr.responseText); }catch(error) {console.log(error);}}; xhr.open ('get', '/version', true); xhr.send(); return false;} setTimeout(getVersion, 300);\n" \
  "function displayMsg(msg) { var element = document.getElementById('msg'); element.innerHTML=msg;}\n""try {  }catch(error) {console.log(error);}"

#define HTML_SETTINGS_JS \
  "function saveSetting(index) { var xhr = new XMLHttpRequest(); xhr.onload = function(){ eval(xhr.responseText); }; xhr.open ('post', '/settings', true); var settingName=document.getElementById('name-' + index); var settingValue=document.getElementById('setting-' + index); var d = new FormData(); d.append('save', 'true'); d.append('index', index);d.append('name', settingName.value); d.append('value', settingValue.value); xhr.send (d); return false;}\n" \
  "function saveSettings() { for(var i = 0; i < arguments.length; i++) { setTimeout(saveSetting, i * 100 + 1, arguments[i]); } }\n" \
  "function addSetting(index, label, name, value, type) { var element = document.createElement('div'); element.id = 'divSet-' + index;" \
  " if (type=='password') { element.innerHTML = label + \": \" + \"<br><input type='password' name='setting-\" + index + \"' id='setting-\" + index + \"' value='\" + value + \"'><input type='hidden' name='name-\" + index + \"' id='name-\" + index + \"' value='\" + name + \"'><button name='btn-\" + index + \"' id='btn-\" + index + \"' onclick='saveSetting(\" + index + \");'>Apply</button>\";}\n" \
  " else {element.innerHTML = label + \": \" + \"<br><input type='text' name='setting-\" + index + \"' id='setting-\" + index + \"' value='\" + value + \"'><input type='hidden' name='name-\" + index + \"' id='name-\" + index + \"' value='\" + name + \"'><button name='btn-\" + index + \"' id='btn-\" + index + \"' onclick='saveSetting(\" + index + \");'>Apply</button>\";}\n" \
  " document.getElementById(\"settings\").appendChild(element);}\n" \
  "function updateSetting(index, label, name, value, type) { var element = document.getElementById(\"setting-\" + index); if (element===null) { addSetting(index, label, name, value, type); element = document.getElementById(\"setting-\" + index); } element.value=value;}\n" \
  "function updateLightBlock(index, label, displayName, topicIndex, startPixel, endPixel, density, transition, powerOnState, conR, conG, conB, coffR, coffG, coffB) { \n" \
  "var element = document.getElementById('lightSet-' + index); " \
  "if (element===null) { " \
  "  element = document.createElement('div');" \
  "  element.style = 'text-align:left;';" \
  "  element.id = 'lightSet-' + index;" \
  "  var fs = document.createElement('fieldset'); " \
  "  var fsLabel = document.createElement('legend'); fsLabel.innerHTML =label; fsLabel.className = 'secTitle'; fs.appendChild(fsLabel); fs.appendChild(element);" \
  "  var settings = document.getElementById('settings');\n" \ 
  "  settings.appendChild(fs);\n" \
  "}\n" \
  "var s = \"\";" \
  "s += \"Display Name: <input type='text' class='lsL' name='displayName-\" + index + \"' id='displayName-\" + index + \"' value='\" + displayName + \"'><br>\";" \
  "s += \"MQTT Topic Id: <input type='text' class='lsS' name='topic-\" + index + \"' id='topic-\" + index + \"' value='\" + topicIndex + \"'> (0 = disabled)<br>\";" \
  "s += \"Power On State : <input type='text' class='lsS' name='powerOnState-\" + index + \"' id='powerOnState-\" + index + \"' value='\" + powerOnState + \"'><br>\";" \
  "s += \"Start Pixel : <input type='text' class='lsS' name='startPixel-\" + index + \"' id='startPixel-\" + index + \"' value='\" + startPixel + \"'> End Pixel : <input type='text' class='lsS' name='endPixel-\" + index + \"' id='endPixel-\" + index + \"' value='\" + endPixel + \"'><br>\";" \
  "s += \"Density : <input type='text' class='lsS' name='density-\" + index + \"' id='density-\" + index + \"' value='\" + density + \"'> " \
  "Transition : " \
  "<select class='lsM' name='transition-\" + index + \"' id='transition-\" + index + \"'><option value='0'>On/Off</option><option value='1'>Cross Fade</option><option value='2'>Wipe Left</option><option value='3'>Wipe Right</option><option value='4'>Expand Center</option></select>" \
  "<br>\";" \
  "s += \"On Color - R:<input type='text' class='lsS' name='conR-\" + index + \"' id='conR-\" + index + \"' value='\" + conR + \"'> G:<input type='text' class='lsS' name='conG-\" + index + \"' id='conG-\" + index + \"' value='\" + conG + \"'> B:<input type='text' class='lsS' name='conB-\" + index + \"' id='conB-\" + index + \"' value='\" + conB + \"'><br>\";" \
  "s += \"Off Color - R:<input type='text' class='lsS' name='coffR-\" + index + \"' id='coffR-\" + index + \"' value='\" + coffR + \"'> G:<input type='text' class='lsS' name='coffG-\" + index + \"' id='coffG-\" + index + \"' value='\" + coffG + \"'> B:<input type='text' class='lsS' name='coffB-\" + index + \"' id='coffB-\" + index + \"' value='\" + coffB + \"'><br>\";" \
  "s += \"<button name='btn-\" + index + \"' id='btn-\" + index + \"' onclick='saveLightBlock(\" + index + \");'>Apply</button>\";\n" \
  "element.innerHTML = s;\n" \
  "document.getElementById('transition-' + index ).options[ transition ].selected = true;}\n" \
  "function saveLightBlock(index) { var xhr = new XMLHttpRequest(); xhr.onload = function(){ eval(xhr.responseText); }; xhr.open('post', '/settings', true);\n" \
  "var d = new FormData();" \
  "d.append('save', 'true');" \
  "d.append('index', index);" \
  "d.append('displayName', document.getElementById('displayName-' + index).value);"  \
  "d.append('topic', document.getElementById('topic-' + index).value);" \
  "d.append('density', document.getElementById('density-' + index).value);" \
  "d.append('transition', document.getElementById('transition-' + index).value);" \
  "d.append('powerOnState', document.getElementById('powerOnState-' + index).value);" \
  "d.append('startPixel', document.getElementById('startPixel-' + index).value);" \
  "d.append('endPixel', document.getElementById('endPixel-' + index).value);" \
  "d.append('conR', document.getElementById('conR-' + index).value);d.append('conG', document.getElementById('conG-' + index).value);d.append('conB', document.getElementById('conB-' + index).value);" \
  "d.append('coffR', document.getElementById('coffR-' + index).value);d.append('coffG', document.getElementById('coffG-' + index).value);d.append('coffB', document.getElementById('coffB-' + index).value);" \
  "xhr.send(d);\n" \
  "return false;\n" \
  "}\n" \
  "function showSNV(section, label, value) { var secId = 'section-' + section.replace(/ /g, '-'); var labelId = secId + '-label-' + label.replace(/ /g, '-'); " \
  "var sec = document.getElementById(secId); " \
  "if (sec===null) { var element = document.createElement('fieldset'); element.id = secId; var titl=document.createElement('legend'); titl.innerHTML = section; titl.className = 'secTitle'; element.appendChild(titl); document.getElementById(\"snv\").appendChild(element); sec = document.getElementById(secId); };" \
  "var lab = document.getElementById(labelId); " \
  "if (lab===null) { var element = document.createElement('div'); element.id = labelId; sec.appendChild(element); lab = document.getElementById(labelId); };" \
  "lab.innerHTML =  '<span class=snvName>' + label + '&nbsp;:&nbsp;</span><span class=snvValue>' + value + '</span>'" \
  "}\n"

#define HTML_HEAD_TITLE_RESOURCES \
  "<meta name=\"viewport\" content = \"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0\">" \
  "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=Edge, chrome=1\"/>" \
  "<link rel=\"icon\" href=\"data:;base64,iVBORw0KGgo=\">" \
  "<script>\n" HTML_COMMON1_JS "\n</script>\n" \
  "<style>\n" HTML_COMMON_CSS "\n</style>" \
  "<title>Led Strip Lights</title>"

// This is the preferred way as it reduces the size of the firmware - but the webserver backlog needs config.
//"<script type=\"text/javascript\" src=\"/res?file=common1.js\"></script>\n" \
//"<link id=\"maincss\" type=\"text/css\" rel=\"stylesheet\" href=\"/res?file=common.css\">\n" \
// This is the non preferred way to add CSS
// "<style>\n" HTML_COMMON_CSS "\n</style>" \


#define HTML_BODY_HEADER \
  "<div style=\"text-align:left;display:inline-block;min-width:340px;\"><div style=\"text-align:center;\"><h3>Led Strip Lights</h3>" \
  "<div id=msgDiv><div id=msg></div></div>"

#define HTML_BODY_FOOTER \
  "</div>" \
  "<div style='text-align:right;font-size:11px;'><hr><a href='https://www.gappleby.com/light/GapLit' target='_blank' style='color:#aaa;'><span id=version></span></a></div>" \
  "</div>"

#define HTML_MENU \
  "<div style='text-align:center;'>" \
  "<a href=\"/\">[ Home ]</a> " \
  "<a href=\"/settings\">[ General Settings ]</a> " \
  "<a href=\"/lightsettings\">[ Light Settings ]</a><br> " \
  "<a href=\"/showmqtt\">[ MQTT Summary ]</a> " \
  "<a href=\"/webupdate\">[ Update Firmware ]</a> " \
  "<a href=\"/reboot\">[ Reboot ]</a>" \
  "</div>"

const char RES_COMMON1_JS[] PROGMEM = HTML_COMMON1_JS;
const char RES_SETTINGS_JS[] PROGMEM = HTML_SETTINGS_JS;
const char RES_COMMON_CSS[] PROGMEM = HTML_COMMON_CSS;

const char INDEX_HTML[] PROGMEM =
  "<!DOCTYPE HTML>\n<html>"
  HTML_HEAD_BEGIN
  HTML_HEAD_TITLE_RESOURCES
  "<script>\n"
  "function refreshLS(){ var xhr = new XMLHttpRequest(); xhr.onload = function(){ eval(xhr.responseText);}; xhr.open ('POST', '/', true); var d = new FormData(); d.append('refresh','true'); xhr.send(d); return false;}\n"
  "if(window.addEventListener){ window.addEventListener('load', function(){ setTimeout(refreshLS, 150);  }) } else { window.attachEvent('onload', function() { setTimeout(refreshLS, 150);  }) }\n"
  "</script>\n"
  HTML_HEAD_END
  "<body>"
  HTML_BODY_HEADER
  "<div id='lt' name='lt'></div>"
  HTML_MENU
  HTML_BODY_FOOTER
  "</body>"
  "</html>";


const char REBOOTING_HTML[] PROGMEM =
  "<!DOCTYPE HTML>"
  "<html>"
  HTML_HEAD_BEGIN
  HTML_HEAD_TITLE_RESOURCES
  "<script>"
  "function processQS(){ var url = window.location.href; if (url.indexOf('reboot=msg') > 0) { displayMsg('Rebooting.... Please wait 20 seconds<br>If fails, wait 60 seconds before restarting'); setTimeout(function(){location.href=\"/\"},20000); }}\n"
  "if(window.addEventListener){ window.addEventListener('load', function(){ setTimeout(processQS, 150);  }) } else { window.attachEvent('onload', function() { setTimeout(processQS, 150); }) }\n"
  "</script>"
  HTML_HEAD_END
  "<body>"
  HTML_BODY_HEADER
  "<div id=rebootDiv><button name='btn-0' id='btn-0' onclick='reboot();'>Reboot</button>\n"
  HTML_MENU
  HTML_BODY_FOOTER
  "</body>"
  "</html>";


const char SHOWMQTT_HTML[] PROGMEM =
  "<!DOCTYPE HTML>"
  "<html>"
  HTML_HEAD_BEGIN
  HTML_HEAD_TITLE_RESOURCES
  "<script>\n" HTML_SETTINGS_JS "\n</script>\n"
  "<script>\n"
  "function refreshSettings() { var xhr = new XMLHttpRequest();  xhr.onload = function(){ eval(xhr.responseText); }; xhr.open ('POST', '/showmqtt', true); var d = new FormData(); d.append('refresh', 'true');  xhr.send(d); return false;}\n"
  "if(window.addEventListener){window.addEventListener('load', function() { setTimeout(refreshSettings, 150);})}else{window.attachEvent('onload', function() { setTimeout(refreshSettings, 150); })};\n"
  "</script>\n"
  HTML_HEAD_END
  "<body>"
  HTML_BODY_HEADER
  "<div id='snv' name='snv'></div>"
  HTML_MENU
  HTML_BODY_FOOTER
  "</body>"
  "</html>";


const char SETTINGS_HTML[] PROGMEM =
  "<!DOCTYPE HTML>"
  "<html>"
  HTML_HEAD_BEGIN
  HTML_HEAD_TITLE_RESOURCES
  "<script>\n" HTML_SETTINGS_JS "\n</script>\n"
// This is the preferred way as it reduces the size of the firmware - but the webserver backlog needs config.
//"<script type=\"text/javascript\" src=\"/res?file=settings.js\"></script>\n"
  "<script>\n"
  "function refreshSettings() { var xhr = new XMLHttpRequest();  xhr.onload = function(){ eval(xhr.responseText); }; xhr.open ('POST', '/settings', true); var d = new FormData(); d.append('refresh', 'true');  xhr.send(d); return false;}\n"
  "if(window.addEventListener){window.addEventListener('load', function() {setTimeout(refreshSettings, 150);})}else{window.attachEvent('onload', function() {setTimeout(refreshSettings, 150)})};\n"
  "</script>\n"
  HTML_HEAD_END
  "<body>"
  HTML_BODY_HEADER
  "<style>fieldset input{width:50%;}\nfieldset{text-align:left;}\ndiv{text-align:right;}</style>"
  "<fieldset><legend class=secTitle>Basic Settings</legend><div>"
  "Hostname Template: <input type='text' name='setting-1' id='setting-1' value='%s-%04d'><input type='hidden' name='name-1' id='name-1' value='hostname'><br>"
  "Admin Web Username: <input type='text' name='setting-24' id='setting-24' value='admin'><input type='hidden' name='name-24' id='name-24' value='web_user'><br>"
  "Admin Web Password: <input type='password' name='setting-25' id='setting-25' value=''><input type='hidden' name='name-25' id='name-25' value='web_pwd'><br>"
  "Serial Debug (0 disable): <input type='text' name='setting-12' id='setting-12' value='0'><input type='hidden' name='name-12' id='name-12' value='seriallog_level'><br>"
  "Max Strip LEDs: <input type='text' name='setting-28' id='setting-28' value='300'><input type='hidden' name='name-28' id='name-28' value='pixels'><br>"
  "Led Strip PIN Out: <input type='text' name='setting-26' id='setting-26' value='5'><input type='hidden' name='name-26' id='name-26' value='ls_gpio'><br>"
  "Status PIN Out: <input type='text' name='setting-27' id='setting-27' value='2'><input type='hidden' name='name-27' id='name-27' value='status_light_gpio'><br>"
  "Relay PIN Out (-1 disable): <input type='text' name='setting-40' id='setting-40' value='2'><input type='hidden' name='name-40' id='name-40' value='relay_gpio'><br>"
  "Relay Start Delay (ms): <input type='text' name='setting-41' id='setting-41' value='2'><input type='hidden' name='name-41' id='name-41' value='relay_start_delay'><br>"
  "Relay Stop Delay (ms): <input type='text' name='setting-42' id='setting-42' value='2'><input type='hidden' name='name-42' id='name-42' value='relay_stop_delay'><br>"
  "<button name='btn-1' id='btn-1' onclick='saveSettings(1,24,25,12,26,27,28,40,41,42);'>Apply</button>"
  "</div></fieldset><br>"
  "<fieldset><legend class=secTitle>Tracer Effect</legend><div>"
  "<style>#tracerRGB input[type=\"text\"] {width:45px;}</style>"
  "Tracer LEDs: <span style='width:50%;display:inline-block;text-align:left'><input type='text' style='width:50px;' name='setting-30' id='setting-30' value='4'><input type='hidden' name='name-30' id='name-30' value='tracerEffectPixels'></span><br>"
  "Tracer Colour: <span id=tracerRGB style='width:50%;display:inline-block;text-align:left'>R<input type='text' name='setting-31' id='setting-31' value='255'><input type='hidden' name='name-31' id='name-31' value='tracerColourR'>&nbsp;G<input type='text' name='setting-32' id='setting-32' value='240'><input type='hidden' name='name-32' id='name-32' value='tracerColourG'>&nbsp;B<input type='text' name='setting-33' id='setting-33' value='255'><input type='hidden' name='name-33' id='name-33' value='tracerColourB'></span>"
  "<button name='btn-33' id='btn-33' onclick='saveSettings(30,31,32,33);'>Apply</button>"
  "</div></fieldset><br>"
  "<fieldset><legend class=secTitle>MQTT Settings</legend><div>"
  "MQTT Enabled: <span style='width:50%;display:inline-block;text-align:left'><input type='text' name='setting-11' id='setting-11' style='width: 40px' value='1'><input type='hidden' name='name-11' id='name-11' value='mqtt_enabled'></span><br>"
  "MQTT Host: <input type='text' name='setting-2' id='setting-2' value='10.0.0.240'><input type='hidden' name='name-2' id='name-2' value='mqtt_host'><br>"
  "MQTT Port: <input type='text' name='setting-3' id='setting-3' value='1883'><input type='hidden' name='name-3' id='name-3' value='mqtt_port'><br>"
  "MQTT Client: <input type='text' name='setting-4' id='setting-4' value='DVES_%06X'><input type='hidden' name='name-4' id='name-4' value='mqtt_client'><br>"
  "MQTT User: <input type='text' name='setting-5' id='setting-5' value='homeassistant'><input type='hidden' name='name-5' id='name-5' value='mqtt_user'><br>"
  "MQTT Password: <input type='password' name='setting-6' id='setting-6' value='funnyone'><input type='hidden' name='name-6' id='name-6' value='mqtt_pwd'><br>"
  "MQTT Topic: <input type='text' name='setting-7' id='setting-7' value='gaplit'><input type='hidden' name='name-7' id='name-7' value='mqtt_topic'><br>"
  "Button Topic: <input type='text' name='setting-8' id='setting-8' value='0'><input type='hidden' name='name-8' id='name-8' value='button_topic'><br>"
  "MQTT Group Topic: <input type='text' name='setting-9' id='setting-9' value='sonoffs'><input type='hidden' name='name-9' id='name-9' value='mqtt_grptopic'><br>"
  "MQTT Retry Seconds: <input type='text' name='setting-10' id='setting-10' value='15'><input type='hidden' name='name-10' id='name-10' value='mqtt_retry'><br>"
  "<button name='btn-2' id='btn-2' onclick='saveSettings(2,3,4,5,6,7,8,9,10,11)'>Apply</button>"
  "</div></fieldset><br>"
  "<div id=storeSettings><input type='hidden' name='setting-0' id='setting-0' value='save'><input type='hidden' name='name-0' id='name-0' value='SaveAll'><button name='btn-0' id='btn-0' onclick='saveSetting(0);'>Save</button><br>\n"
  HTML_MENU
  HTML_BODY_FOOTER
  "</body>"
  "</html>";



const char LIGHT_SETTINGS_HTML[] PROGMEM =
  "<!DOCTYPE HTML>"
  "<html>"
  HTML_HEAD_BEGIN
  HTML_HEAD_TITLE_RESOURCES
  "<script>\n" HTML_SETTINGS_JS "\n</script>\n"
  "<script>\n"
  "var formUrl = \"/settings\";\n"
  "var formMethod = 'POST';\n"
  "function refreshLightSettings() { var xhr = new XMLHttpRequest(); xhr.onload = function(){ eval(xhr.responseText); }; xhr.open ('post', '/lightsettings', true); var d = new FormData(); d.append('refresh', 'true');  xhr.send(d); return false; }\n"
  "if(window.addEventListener){window.addEventListener('load', function() {setTimeout(refreshLightSettings, 150);})}else{window.attachEvent('onload', function() {setTimeout(refreshLightSettings, 150);})};\n"
  "</script>\n"
  HTML_HEAD_END
  "<body>"
  HTML_BODY_HEADER
  "<div id='settings' name='settings'></div>"
  "<div id=storeSettings><input type='hidden' name='setting-0' id='setting-0' value='save'><input type='hidden' name='name-0' id='name-0' value='SaveAll'><button name='btn-0' id='btn-0' onclick='saveSetting(0);'>Save</button><br>\n"
  HTML_MENU
  HTML_BODY_FOOTER
  "</body>"
  "</html>";


const char LIGHT_BULB_ON_SVG[] PROGMEM =
  "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 151.84 246.45\">"
  "<title>bulb</title>"
  "<path d=\"M75.92,3a72.92,72.92,0,0,0-57,118.39C24.12,129.3,35,147.31,37.2,162.27c1.35,9.24-.45,21.19,1.86,26.72,1.22,2.91,4,6,10.24,8.1l57.76-3,0.65,0,0.8,0a12,12,0,0,0,4.25-5.15c2.32-5.53.52-17.48,1.86-26.72,2.18-15,13.09-33,18.29-40.89A72.92,72.92,0,0,0,75.92,3ZM128.8,118.09l-0.15.19-0.13.2c-6.16,9.36-16.83,27.4-19.11,43a94.46,94.46,0,0,0-.62,13.57c0,4.56,0,9.73-.88,11.87a6.1,6.1,0,0,1-1.31,1.93l-56.51,2.89a14.74,14.74,0,0,1-4.36-2.43,6.38,6.38,0,0,1-1.8-2.39C43,184.81,43,179.64,43,175.08a94.58,94.58,0,0,0-.62-13.57c-2.27-15.63-12.95-33.67-19.11-43l-0.13-.2L23,118.09A67.68,67.68,0,1,1,128.8,118.09Z\" style=\"fill:#5cff00;stroke:#5cff60;stroke-miterlimit:10;stroke-width:6px\"/>"
  "<path d=\"M63.61,240.67a16.68,16.68,0,0,0,12.63,5.78h0a16.68,16.68,0,0,0,12.65-5.81l2.34-2.72h-30Z\" style=\"fill:#1c8f00\"/>"
  "<path d=\"M54.88,234.69H97.59c4.6,0,8.33-4.48,8.33-10v0l-58.74,3.86C48.43,232.14,51.4,234.69,54.88,234.69Z\" style=\"fill:#1c8f00\"/>"
  "<path d=\"M107.23,199.79l-62.54,3.94c-3.12.2-5.58,2.22-5.48,4.51s2.7,4,5.83,3.8l62.54-3.94c3.12-.2,5.58-2.22,5.48-4.51S110.35,199.6,107.23,199.79Z\" style=\"fill:#1c8f00\"/>"
  "<path d=\"M104.82,212.07L46.69,216c-2.9.2-5.18,2.22-5.1,4.51s2.51,4,5.42,3.8l58.13-3.94c2.9-.2,5.18-2.22,5.1-4.51S107.73,211.87,104.82,212.07Z\" style=\"fill:#1c8f00\"/>"
  "</svg>";

const char LIGHT_BULB_OFF_SVG[] PROGMEM =
  "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 151.84 246.45\">"
  "<title>bulb</title>"
  "<path d=\"M75.92,3a72.92,72.92,0,0,0-57,118.39C24.12,129.3,35,147.31,37.2,162.27c1.35,9.24-.45,21.19,1.86,26.72,1.22,2.91,4,6,10.24,8.1l57.76-3,0.65,0,0.8,0a12,12,0,0,0,4.25-5.15c2.32-5.53.52-17.48,1.86-26.72,2.18-15,13.09-33,18.29-40.89A72.92,72.92,0,0,0,75.92,3ZM128.8,118.09l-0.15.19-0.13.2c-6.16,9.36-16.83,27.4-19.11,43a94.46,94.46,0,0,0-.62,13.57c0,4.56,0,9.73-.88,11.87a6.1,6.1,0,0,1-1.31,1.93l-56.51,2.89a14.74,14.74,0,0,1-4.36-2.43,6.38,6.38,0,0,1-1.8-2.39C43,184.81,43,179.64,43,175.08a94.58,94.58,0,0,0-.62-13.57c-2.27-15.63-12.95-33.67-19.11-43l-0.13-.2L23,118.09A67.68,67.68,0,1,1,128.8,118.09Z\" style=\"fill:#5c5d60;stroke:#5c5d60;stroke-miterlimit:10;stroke-width:6px\"/>"
  "<path d=\"M63.61,240.67a16.68,16.68,0,0,0,12.63,5.78h0a16.68,16.68,0,0,0,12.65-5.81l2.34-2.72h-30Z\" style=\"fill:#5c5d60\"/>"
  "<path d=\"M54.88,234.69H97.59c4.6,0,8.33-4.48,8.33-10v0l-58.74,3.86C48.43,232.14,51.4,234.69,54.88,234.69Z\" style=\"fill:#5c5d60\"/>"
  "<path d=\"M107.23,199.79l-62.54,3.94c-3.12.2-5.58,2.22-5.48,4.51s2.7,4,5.83,3.8l62.54-3.94c3.12-.2,5.58-2.22,5.48-4.51S110.35,199.6,107.23,199.79Z\" style=\"fill:#5c5d60\"/>"
  "<path d=\"M104.82,212.07L46.69,216c-2.9.2-5.18,2.22-5.1,4.51s2.51,4,5.42,3.8l58.13-3.94c2.9-.2,5.18-2.22,5.1-4.51S107.73,211.87,104.82,212.07Z\" style=\"fill:#5c5d60\"/>"
  "</svg>";

const char CONTENT_TYPE_HTML_P[]  = "text/html";
const char CONTENT_TYPE_HTML[] = "text/html";
const char CONTENT_TYPE_TEXT[] = "text/plain";
const char CONTENT_TYPE_JSON[] = "application/json";
const char CONTENT_TYPE_SVG_P[]  = "image/svg+xml";
const char CONTENT_TYPE_CSS_P[]  = "text/css";
const char CONTENT_TYPE_JS_P[]  = "text/javascript";
const char CONTENT_TYPE_JS[] = "text/javascript";

const char FILE_NOT_FOUND_HTML[] PROGMEM =
  "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">"
  "<html><head>"
  "<title>404 Not Found</title>"
  "</head><body>Oops</body></html>";

const char OUT_OF_MEMORY[] PROGMEM =
  "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">"
  "<html><head>"
  "<title>404 Not Found</title>"
  "</head><body>Low on memory</body></html>";


const char UPDATE_PAGE_HTML[] =
  "<!DOCTYPE HTML>"
  "<html>"
  HTML_HEAD_BEGIN
  HTML_HEAD_TITLE_RESOURCES
  "<script>\n"
  "</script>\n"
  HTML_HEAD_END
  "<body>"
  HTML_BODY_HEADER
  "<form id='uploader' method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update' id='update' class='inputfile'><label for='update'>Choose a file</label></form><button name='btn-upload' id='btn-upload' onclick='document.getElementById(\"uploader\").submit();'>Update Firmware</button>"
  HTML_MENU
  HTML_BODY_FOOTER
  "</body>"
  "</html>";


#endif  // __LocalWebsite_lang_H_

