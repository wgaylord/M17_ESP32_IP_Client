/*
 Created:	1-Nov-2021 14:27:23
 Author:	HS5TQA/Atten
*/

#include "webservice.h"
#include "m17.h"
#include "I2S.h"

extern hw_timer_t *timer;
extern char current_module;

// Web Server;
WebServer server(80);

String webString;

bool defaultSetting = false;

void serviceHandle()
{
	server.handleClient();
}
void setHTML()
{
	webString = "<html><head>\n";
	webString += "<meta charset=\"utf-8\">";
	webString += "<style>\nhdr1{background-color: powderblue;color: white;vertical-align: middle;text-align: center;font-size: 16px;font-weight: bold;}\n</style>\n";
	webString += "<meta http - equiv = \"content-type\" content = \"text/html; charset=utf-8\" / > \n";
	webString += "<style>\n"
				 ".topnav{"
				 "position:relative;"
				 "z-index:2;"
				 "font-size:25px;"
				 "background-color:#5f5f5f;"
				 "color:#f1f1f1;"
				 "width:100%;"
				 "padding:10px;"
				 "letter-spacing:3px;"
				 "box-shadow:0 10px 10px 0 rgba(0,0,0,0.16);"
				 "font-family:\"Segoe UI\",Tahoma,sans-serif;"
				 "}\n"
				 ".title_hdr{"
				 "text-align: center;"
				 //"width: auto;"
				 "margin: 0 auto;"
				 "background: darkblue;"
				 "color: white;"
				 "font-size: 10px;"
				 "border-radius: 5px;"
				 "}\n"
				 ".title_value{"
				 "width: auto;"
				 "margin: 0 auto;"
				 "text-align: center;"
				 "font-size: 14px;"
				 "color: darkgreen;"
				 "background: white;"
				 "border-radius: 5px;"
				 "}\n"
				 ".L1{"
				 "text-align: center;"
				 "width: 33%;"
				 "margin: 1px;"
				 "background: red;"
				 "color: sandybrown;"
				 "font-size: 10px;"
				 "border-radius: 5px;"
				 "font-weight: bold;"
				 "}\n"
				 ".L2{"
				 "text-align: center;"
				 "width: 33%;"
				 "margin: 1px;"
				 "background: yellow;"
				 "color: black;"
				 "font-size: 10px;"
				 "border-radius: 5px;"
				 "font-weight: bold;"
				 "}\n"
				 ".L3{"
				 "text-align: center;"
				 "width: 33%;"
				 "margin: 1px;"
				 "background: blue;"
				 "color: lightgray;"
				 "font-size: 10px;"
				 "border-radius: 5px;"
				 "font-weight: bold;"
				 "}\n";
	webString += F(".col-pad{width: 500px;}");
	webString += F(".form-control{display:block;width:100%;height:34px;padding:6px 12px;font-size:14px;line-height:1.42857143;color:#555;background-color:#fff;background-image:none;border:1px solid #ccc;border-radius:4px;-webkit-box-shadow:inset 0 1px 1px rgba(0,0,0,.075);box-shadow:inset 0 1px 1px rgba(0,0,0,.075);-webkit-transition:border-color ease-in-out .15s,box-shadow ease-in-out .15s;transition:border-color ease-in-out .15s,box-shadow ease-in-out .15s}");
	webString += F(".btn{display:inline-block;margin-bottom:0;font-weight:400;text-align:center;vertical-align:middle;cursor:pointer;background-image:none;border:1px solid transparent;white-space:nowrap;padding:6px 12px;font-size:14px;line-height:1.42857143;border-radius:4px;-webkit-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none}.btn:focus,.btn:active:focus,.btn.active:focus{outline:thin dotted;outline:5px auto -webkit-focus-ring-color;outline-offset:-2px}.btn:hover,.btn:focus{color:#333;text-decoration:none}.btn:active,.btn.active{outline:0;background-image:none;-webkit-box-shadow:inset 0 3px 5px rgba(0,0,0,.125);box-shadow:inset 0 3px 5px rgba(0,0,0,.125)}.btn.disabled,.btn[disabled],fieldset[disabled] .btn{cursor:not-allowed;pointer-events:none;opacity:.65;filter:alpha(opacity=65);-webkit-box-shadow:none;box-shadow:none}.btn-default{color:#333;background-color:#fff;border-color:#ccc}.btn-default:hover,.btn-default:focus,.btn-default:active,.btn-default.active,.open .dropdown-toggle.btn-default{color:#333;background-color:#ebebeb;border-color:#adadad}.btn-default:active,.btn-default.active,.open .dropdown-toggle.btn-default{background-image:none}.btn-default.disabled,.btn-default[disabled],fieldset[disabled] .btn-default,.btn-default.disabled:hover,.btn-default[disabled]:hover,fieldset[disabled] .btn-default:hover,.btn-default.disabled:focus,.btn-default[disabled]:focus,fieldset[disabled] .btn-default:focus,.btn-default.disabled:active,.btn-default[disabled]:active,fieldset[disabled] .btn-default:active,.btn-default.disabled.active,.btn-default[disabled].active,fieldset[disabled] .btn-default.active{background-color:#fff;border-color:#ccc}.btn-default .badge{color:#fff;background-color:#333}");
	webString += F(".btn-danger {color: #fff;background-color: #d9534f;border-color: #d43f3a;}");
	webString += F(".btn-primary {color: #fff;background-color: #428bca;border-color: #357ebd;}");
	webString += F(".clearfix:after, .container:after, .container-fluid:after, .row:after, .form-horizontal .form-group:after, .btn-toolbar:after, .btn-group-vertical>.btn-group:after, .nav:after, .navbar:after, .navbar-header:after, .navbar-collapse:after, .pager:after, .panel-body:after, .modal-footer:after {clear: both;}");
	webString += F(".clearfix:before, .clearfix:after, .container:before, .container:after, .container-fluid:before, .container-fluid:after, .row:before, .row:after, .form-horizontal .form-group:before, .form-horizontal .form-group:after, .btn-toolbar:before, .btn-toolbar:after, .btn-group-vertical>.btn-group:before, .btn-group-vertical>.btn-group:after, .nav:before, .nav:after, .navbar:before, .navbar:after, .navbar-header:before, .navbar-header:after, .navbar-collapse:before, .navbar-collapse:after, .pager:before, .pager:after, .panel-body:before, .panel-body:after, .modal-footer:before, .modal-footer:after {content: \" \";display: table;}");
	webString += F(".nav{margin-bottom:0;padding-left:0;list-style:none}.nav>li{position:relative;display:block}.nav>li>a{position:relative;display:block;padding:10px 15px}.nav>li>a:hover,.nav>li>a:focus{text-decoration:none;background-color:#eee}.nav>li.disabled>a{color:#999}.nav>li.disabled>a:hover,.nav>li.disabled>a:focus{color:#999;text-decoration:none;background-color:transparent;cursor:not-allowed}.nav .open>a,.nav .open>a:hover,.nav .open>a:focus{background-color:#eee;border-color:#428bca}.nav .nav-divider{height:1px;margin:9px 0;overflow:hidden;background-color:#e5e5e5}.nav>li>a>img{max-width:none}.nav-tabs{border-bottom:1px solid #ddd}.nav-tabs>li{float:left;margin-bottom:-1px}.nav-tabs>li>a{margin-right:0px;line-height:1.42857143;border:1px solid #ddd;border-radius:10px 10px 0 0}.nav-tabs>li>a:hover{border-color:#eee #eee #ddd}.nav-tabs>li.active>a,.nav-tabs>li.active>a:hover,.nav-tabs>li.active>a:focus{color:#428bca;background-color:#e5e5e5;border:1px solid #ddd;border-bottom-color:transparent;cursor:default}.nav-tabs.nav-justified{width:100%;border-bottom:0}.nav-tabs.nav-justified>li{float:none}.nav-tabs.nav-justified>li>a{text-align:center;margin-bottom:5px}.nav-tabs.nav-justified>.dropdown .dropdown-menu{top:auto;left:auto}@media (min-width:768px){.nav-tabs.nav-justified>li{display:table-cell;width:1%}.nav-tabs.nav-justified>li>a{margin-bottom:0}}.nav-tabs.nav-justified>li>a{margin-right:0;border-radius:4px}.nav-tabs.nav-justified>.active>a,.nav-tabs.nav-justified>.active>a:hover,.nav-tabs.nav-justified>.active>a:focus{border:1px solid #ddd}@media (min-width:768px){.nav-tabs.nav-justified>li>a{border-bottom:1px solid #ddd;border-radius:4px 4px 0 0}.nav-tabs.nav-justified>.active>a,.nav-tabs.nav-justified>.active>a:hover,.nav-tabs.nav-justified>.active>a:focus{border-bottom-color:#fff}}.nav-pills>li{float:left}.nav-pills>li>a{border-radius:4px}.nav-pills>li+li{margin-left:2px}.nav-pills>li.active>a,.nav-pills>li.active>a:hover,.nav-pills>li.active>a:focus{color:#fff;background-color:#428bca}.nav-stacked>li{float:none}.nav-stacked>li+li{margin-top:2px;margin-left:0}.nav-justified{width:100%}.nav-justified>li{float:none}.nav-justified>li>a{text-align:center;margin-bottom:5px}.nav-justified>.dropdown .dropdown-menu{top:auto;left:auto}@media (min-width:768px){.nav-justified>li{display:table-cell;width:1%}.nav-justified>li>a{margin-bottom:0}}.nav-tabs-justified{border-bottom:0}.nav-tabs-justified>li>a{margin-right:0;border-radius:4px}.nav-tabs-justified>.active>a,.nav-tabs-justified>.active>a:hover,.nav-tabs-justified>.active>a:focus{border:1px solid #ddd}@media (min-width:768px){.nav-tabs-justified>li>a{border-bottom:1px solid #ddd;border-radius:4px 4px 0 0}.nav-tabs-justified>.active>a,.nav-tabs-justified>.active>a:hover,.nav-tabs-justified>.active>a:focus{border-bottom-color:#fff}}.tab-content>.tab-pane{display:none}.tab-content>.active{display:block}.nav-tabs .dropdown-menu{margin-top:-1px;border-top-right-radius:0;border-top-left-radius:0}");
	webString += F(".form-group{margin-bottom:15px}.radio,.checkbox{display:block;min-height:20px;margin-top:10px;margin-bottom:10px;padding-left:20px}.radio label,.checkbox label{display:inline;font-weight:400;cursor:pointer}.radio input[type=radio],.radio-inline input[type=radio],.checkbox input[type=checkbox],.checkbox-inline input[type=checkbox]{float:left;margin-left:-20px}.radio+.radio,.checkbox+.checkbox{margin-top:-5px}.radio-inline,.checkbox-inline{display:inline-block;padding-left:20px;margin-bottom:0;vertical-align:middle;font-weight:400;cursor:pointer}.radio-inline+.radio-inline,.checkbox-inline+.checkbox-inline{margin-top:0;margin-left:10px}input[type=radio][disabled],input[type=checkbox][disabled],.radio[disabled],.radio-inline[disabled],.checkbox[disabled],.checkbox-inline[disabled],fieldset[disabled] input[type=radio],fieldset[disabled] input[type=checkbox],fieldset[disabled] .radio,fieldset[disabled] .radio-inline,fieldset[disabled] .checkbox,fieldset[disabled] .checkbox-inline{cursor:not-allowed}.input-sm{height:30px;padding:5px 10px;font-size:12px;line-height:1.5;border-radius:3px}select.input-sm{height:30px;line-height:30px}textarea.input-sm,select[multiple].input-sm{height:auto}.input-lg{height:46px;padding:10px 16px;font-size:18px;line-height:1.33;border-radius:6px}select.input-lg{height:46px;line-height:46px}textarea.input-lg,select[multiple].input-lg{height:auto}.has-feedback{position:relative}.has-feedback .form-control{padding-right:42.5px}.has-feedback .form-control-feedback{position:absolute;top:25px;right:0;display:block;width:34px;height:34px;line-height:34px;text-align:center}.has-success .help-block,.has-success .control-label,.has-success .radio,.has-success .checkbox,.has-success .radio-inline,.has-success .checkbox-inline{color:#3c763d}.has-success .form-control{border-color:#3c763d;-webkit-box-shadow:inset 0 1px 1px rgba(0,0,0,.075);box-shadow:inset 0 1px 1px rgba(0,0,0,.075)}.has-success .form-control:focus{border-color:#2b542c;-webkit-box-shadow:inset 0 1px 1px rgba(0,0,0,.075),0 0 6px #67b168;box-shadow:inset 0 1px 1px rgba(0,0,0,.075),0 0 6px #67b168}.has-success .input-group-addon{color:#3c763d;border-color:#3c763d;background-color:#dff0d8}.has-success .form-control-feedback{color:#3c763d}.has-warning .help-block,.has-warning .control-label,.has-warning .radio,.has-warning .checkbox,.has-warning .radio-inline,.has-warning .checkbox-inline{color:#8a6d3b}.has-warning .form-control{border-color:#8a6d3b;-webkit-box-shadow:inset 0 1px 1px rgba(0,0,0,.075);box-shadow:inset 0 1px 1px rgba(0,0,0,.075)}.has-warning .form-control:focus{border-color:#66512c;-webkit-box-shadow:inset 0 1px 1px rgba(0,0,0,.075),0 0 6px #c0a16b;box-shadow:inset 0 1px 1px rgba(0,0,0,.075),0 0 6px #c0a16b}.has-warning .input-group-addon{color:#8a6d3b;border-color:#8a6d3b;background-color:#fcf8e3}.has-warning .form-control-feedback{color:#8a6d3b}.has-error .help-block,.has-error .control-label,.has-error .radio,.has-error .checkbox,.has-error .radio-inline,.has-error .checkbox-inline{color:#a94442}.has-error .form-control{border-color:#a94442;-webkit-box-shadow:inset 0 1px 1px rgba(0,0,0,.075);box-shadow:inset 0 1px 1px rgba(0,0,0,.075)}.has-error .form-control:focus{border-color:#843534;-webkit-box-shadow:inset 0 1px 1px rgba(0,0,0,.075),0 0 6px #ce8483;box-shadow:inset 0 1px 1px rgba(0,0,0,.075),0 0 6px #ce8483}.has-error .input-group-addon{color:#a94442;border-color:#a94442;background-color:#f2dede}.has-error .form-control-feedback{color:#a94442}.form-control-static{margin-bottom:0}.help-block{display:block;margin-top:5px;margin-bottom:10px;color:#737373}@media (min-width:768px){.form-inline .form-group{display:inline-block;margin-bottom:0;vertical-align:middle}.form-inline .form-control{display:inline-block;width:auto;vertical-align:middle}.form-inline .input-group>.form-control{width:100%}.form-inline .control-label{margin-bottom:0;vertical-align:middle}.form-inline .radio,.form-inline .checkbox{display:inline-block;margin-top:0;margin-bottom:0;padding-left:0;vertical-align:middle}.form-inline .radio input[type=radio],.form-inline .checkbox input[type=checkbox]{float:none;margin-left:0}.form-inline .has-feedback .form-control-feedback{top:0}}.form-horizontal .control-label,.form-horizontal .radio,.form-horizontal .checkbox,.form-horizontal .radio-inline,.form-horizontal .checkbox-inline{margin-top:0;margin-bottom:0;padding-top:0px}.form-horizontal .radio,.form-horizontal .checkbox{min-height:27px}.form-horizontal .form-group{margin-left:-15px;margin-right:-15px}.form-horizontal .form-control-static{padding-top:7px}@media (min-width:768px){.form-horizontal .control-label{text-align:right}}.form-horizontal .has-feedback .form-control-feedback{top:0;right:15px}");
	webString += F(".col-xs-1,.col-sm-1,.col-md-1,.col-lg-1,.col-xs-2,.col-sm-2,.col-md-2,.col-lg-2,.col-xs-3,.col-sm-3,.col-md-3,.col-lg-3,.col-xs-4,.col-sm-4,.col-md-4,.col-lg-4,.col-xs-5,.col-sm-5,.col-md-5,.col-lg-5,.col-xs-6,.col-sm-6,.col-md-6,.col-lg-6,.col-xs-7,.col-sm-7,.col-md-7,.col-lg-7,.col-xs-8,.col-sm-8,.col-md-8,.col-lg-8,.col-xs-9,.col-sm-9,.col-md-9,.col-lg-9,.col-xs-10,.col-sm-10,.col-md-10,.col-lg-10,.col-xs-11,.col-sm-11,.col-md-11,.col-lg-11,.col-xs-12,.col-sm-12,.col-md-12,.col-lg-12{position:relative;min-height:1px;padding-left:15px;padding-right:15px}.col-xs-1,.col-xs-2,.col-xs-3,.col-xs-4,.col-xs-5,.col-xs-6,.col-xs-7,.col-xs-8,.col-xs-9,.col-xs-10,.col-xs-11,.col-xs-12{float:left}.col-xs-12{width:100%}.col-xs-11{width:91.66666667%}.col-xs-10{width:83.33333333%}.col-xs-9{width:75%}.col-xs-8{width:66.66666667%}.col-xs-7{width:58.33333333%}.col-xs-6{width:50%}.col-xs-5{width:41.66666667%}.col-xs-4{width:33.33333333%}.col-xs-3{width:25%}.col-xs-2{width:16.66666667%}.col-xs-1{width:8.33333333%}.col-xs-pull-12{right:100%}.col-xs-pull-11{right:91.66666667%}.col-xs-pull-10{right:83.33333333%}.col-xs-pull-9{right:75%}.col-xs-pull-8{right:66.66666667%}.col-xs-pull-7{right:58.33333333%}.col-xs-pull-6{right:50%}.col-xs-pull-5{right:41.66666667%}.col-xs-pull-4{right:33.33333333%}.col-xs-pull-3{right:25%}.col-xs-pull-2{right:16.66666667%}.col-xs-pull-1{right:8.33333333%}.col-xs-pull-0{right:0}.col-xs-push-12{left:100%}.col-xs-push-11{left:91.66666667%}.col-xs-push-10{left:83.33333333%}.col-xs-push-9{left:75%}.col-xs-push-8{left:66.66666667%}.col-xs-push-7{left:58.33333333%}.col-xs-push-6{left:50%}.col-xs-push-5{left:41.66666667%}.col-xs-push-4{left:33.33333333%}.col-xs-push-3{left:25%}.col-xs-push-2{left:16.66666667%}.col-xs-push-1{left:8.33333333%}.col-xs-push-0{left:0}.col-xs-offset-12{margin-left:100%}.col-xs-offset-11{margin-left:91.66666667%}.col-xs-offset-10{margin-left:83.33333333%}.col-xs-offset-9{margin-left:75%}.col-xs-offset-8{margin-left:66.66666667%}.col-xs-offset-7{margin-left:58.33333333%}.col-xs-offset-6{margin-left:50%}.col-xs-offset-5{margin-left:41.66666667%}.col-xs-offset-4{margin-left:33.33333333%}.col-xs-offset-3{margin-left:25%}.col-xs-offset-2{margin-left:16.66666667%}.col-xs-offset-1{margin-left:8.33333333%}.col-xs-offset-0{margin-left:0}@media (min-width:768px){.col-sm-1,.col-sm-2,.col-sm-3,.col-sm-4,.col-sm-5,.col-sm-6,.col-sm-7,.col-sm-8,.col-sm-9,.col-sm-10,.col-sm-11,.col-sm-12{float:left}.col-sm-12{width:100%}.col-sm-11{width:91.66666667%}.col-sm-10{width:83.33333333%}.col-sm-9{width:75%}.col-sm-8{width:66.66666667%}.col-sm-7{width:58.33333333%}.col-sm-6{width:50%}.col-sm-5{width:41.66666667%}.col-sm-4{width:33.33333333%}.col-sm-3{width:25%}.col-sm-2{width:16.66666667%}.col-sm-1{width:8.33333333%}.col-sm-pull-12{right:100%}.col-sm-pull-11{right:91.66666667%}.col-sm-pull-10{right:83.33333333%}.col-sm-pull-9{right:75%}.col-sm-pull-8{right:66.66666667%}.col-sm-pull-7{right:58.33333333%}.col-sm-pull-6{right:50%}.col-sm-pull-5{right:41.66666667%}.col-sm-pull-4{right:33.33333333%}.col-sm-pull-3{right:25%}.col-sm-pull-2{right:16.66666667%}.col-sm-pull-1{right:8.33333333%}.col-sm-pull-0{right:0}.col-sm-push-12{left:100%}.col-sm-push-11{left:91.66666667%}.col-sm-push-10{left:83.33333333%}.col-sm-push-9{left:75%}.col-sm-push-8{left:66.66666667%}.col-sm-push-7{left:58.33333333%}.col-sm-push-6{left:50%}.col-sm-push-5{left:41.66666667%}.col-sm-push-4{left:33.33333333%}.col-sm-push-3{left:25%}.col-sm-push-2{left:16.66666667%}.col-sm-push-1{left:8.33333333%}.col-sm-push-0{left:0}.col-sm-offset-12{margin-left:100%}.col-sm-offset-11{margin-left:91.66666667%}.col-sm-offset-10{margin-left:83.33333333%}.col-sm-offset-9{margin-left:75%}.col-sm-offset-8{margin-left:66.66666667%}.col-sm-offset-7{margin-left:58.33333333%}.col-sm-offset-6{margin-left:50%}.col-sm-offset-5{margin-left:41.66666667%}.col-sm-offset-4{margin-left:33.33333333%}.col-sm-offset-3{margin-left:25%}.col-sm-offset-2{margin-left:16.66666667%}.col-sm-offset-1{margin-left:8.33333333%}.col-sm-offset-0{margin-left:0}}@media (min-width:992px){.col-md-1,.col-md-2,.col-md-3,.col-md-4,.col-md-5,.col-md-6,.col-md-7,.col-md-8,.col-md-9,.col-md-10,.col-md-11,.col-md-12{float:left}.col-md-12{width:100%}.col-md-11{width:91.66666667%}.col-md-10{width:83.33333333%}.col-md-9{width:75%}.col-md-8{width:66.66666667%}.col-md-7{width:58.33333333%}.col-md-6{width:50%}.col-md-5{width:41.66666667%}.col-md-4{width:33.33333333%}.col-md-3{width:25%}.col-md-2{width:16.66666667%}.col-md-1{width:8.33333333%}.col-md-pull-12{right:100%}.col-md-pull-11{right:91.66666667%}.col-md-pull-10{right:83.33333333%}.col-md-pull-9{right:75%}.col-md-pull-8{right:66.66666667%}.col-md-pull-7{right:58.33333333%}.col-md-pull-6{right:50%}.col-md-pull-5{right:41.66666667%}.col-md-pull-4{right:33.33333333%}.col-md-pull-3{right:25%}.col-md-pull-2{right:16.66666667%}.col-md-pull-1{right:8.33333333%}.col-md-pull-0{right:0}.col-md-push-12{left:100%}.col-md-push-11{left:91.66666667%}.col-md-push-10{left:83.33333333%}.col-md-push-9{left:75%}.col-md-push-8{left:66.66666667%}.col-md-push-7{left:58.33333333%}.col-md-push-6{left:50%}.col-md-push-5{left:41.66666667%}.col-md-push-4{left:33.33333333%}.col-md-push-3{left:25%}.col-md-push-2{left:16.66666667%}.col-md-push-1{left:8.33333333%}.col-md-push-0{left:0}.col-md-offset-12{margin-left:100%}.col-md-offset-11{margin-left:91.66666667%}.col-md-offset-10{margin-left:83.33333333%}.col-md-offset-9{margin-left:75%}.col-md-offset-8{margin-left:66.66666667%}.col-md-offset-7{margin-left:58.33333333%}.col-md-offset-6{margin-left:50%}.col-md-offset-5{margin-left:41.66666667%}.col-md-offset-4{margin-left:33.33333333%}.col-md-offset-3{margin-left:25%}.col-md-offset-2{margin-left:16.66666667%}.col-md-offset-1{margin-left:8.33333333%}.col-md-offset-0{margin-left:0}}@media (min-width:1200px){.col-lg-1,.col-lg-2,.col-lg-3,.col-lg-4,.col-lg-5,.col-lg-6,.col-lg-7,.col-lg-8,.col-lg-9,.col-lg-10,.col-lg-11,.col-lg-12{float:left}.col-lg-12{width:100%}.col-lg-11{width:91.66666667%}.col-lg-10{width:83.33333333%}.col-lg-9{width:75%}.col-lg-8{width:66.66666667%}.col-lg-7{width:58.33333333%}.col-lg-6{width:50%}.col-lg-5{width:41.66666667%}.col-lg-4{width:33.33333333%}.col-lg-3{width:25%}.col-lg-2{width:16.66666667%}.col-lg-1{width:8.33333333%}.col-lg-pull-12{right:100%}.col-lg-pull-11{right:91.66666667%}.col-lg-pull-10{right:83.33333333%}.col-lg-pull-9{right:75%}.col-lg-pull-8{right:66.66666667%}.col-lg-pull-7{right:58.33333333%}.col-lg-pull-6{right:50%}.col-lg-pull-5{right:41.66666667%}.col-lg-pull-4{right:33.33333333%}.col-lg-pull-3{right:25%}.col-lg-pull-2{right:16.66666667%}.col-lg-pull-1{right:8.33333333%}.col-lg-pull-0{right:0}.col-lg-push-12{left:100%}.col-lg-push-11{left:91.66666667%}.col-lg-push-10{left:83.33333333%}.col-lg-push-9{left:75%}.col-lg-push-8{left:66.66666667%}.col-lg-push-7{left:58.33333333%}.col-lg-push-6{left:50%}.col-lg-push-5{left:41.66666667%}.col-lg-push-4{left:33.33333333%}.col-lg-push-3{left:25%}.col-lg-push-2{left:16.66666667%}.col-lg-push-1{left:8.33333333%}.col-lg-push-0{left:0}.col-lg-offset-12{margin-left:100%}.col-lg-offset-11{margin-left:91.66666667%}.col-lg-offset-10{margin-left:83.33333333%}.col-lg-offset-9{margin-left:75%}.col-lg-offset-8{margin-left:66.66666667%}.col-lg-offset-7{margin-left:58.33333333%}.col-lg-offset-6{margin-left:50%}.col-lg-offset-5{margin-left:41.66666667%}.col-lg-offset-4{margin-left:33.33333333%}.col-lg-offset-3{margin-left:25%}.col-lg-offset-2{margin-left:16.66666667%}.col-lg-offset-1{margin-left:8.33333333%}.col-lg-offset-0{margin-left:0}}");
	// webString += "<link rel=\"stylesheet\" href=\"//maxcdn.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap.min.css\" />\n";
	webString += "</style>\n";

	String strActiveP1 = "";
	String strActiveP2 = "";
	String strActiveP3 = "";
	String strActiveP4 = "";
	String strActiveP5 = "";
	String strActiveP6 = "";
	String strActiveP7 = "";
	String strActiveP8 = "";
	webString += "</head><body>\n";
	webString += "<div class='w3-card-2 topnav notranslate' id='topnav'><b>M17 Analog Gateway</div>\n";
	// webString += "</body></html>\n";
}

////////////////////////////////////////////////////////////
// handler for web server request: http://IpAddress/      //
////////////////////////////////////////////////////////////

void handle_setting()
{
	bool wifiSTA = false;
	bool wifiAP = false;
	bool oledEN = false;
	bool aprsEn = false;

	if (defaultSetting)
	{
		disconnect_from_host(); // Restart M17 to Reflector
		defaultConfig();
	}
	else
	{
		if (server.args() > 0)
		{
			// #ifndef I2S_INTERNAL
			// 			bool adcIsr = timerAlarmEnabled(timer);
			// 			if (adcIsr)
			// 				timerAlarmDisable(timer);
			// #endif
			disconnect_from_host(); // Restart M17 to Reflector
			for (uint8_t i = 0; i < server.args(); i++)
			{
				// Serial.print("SERVER ARGS ");
				// Serial.print(server.argName(i));
				// Serial.print("=");
				// Serial.println(server.arg(i));
				if (server.argName(i) == "aprsEnable")
				{
					if (server.arg(i) != "")
					{
						if (String(server.arg(i)) == "OK")
						{
							aprsEn = true;
						}
					}
				}

				if (server.argName(i) == "oledEnable")
				{
					if (server.arg(i) != "")
					{
						if (String(server.arg(i)) == "OK")
						{
							oledEN = true;
						}
					}
				}

				if (server.argName(i) == "wifiAP")
				{
					if (server.arg(i) != "")
					{
						if (String(server.arg(i)) == "OK")
						{
							wifiAP = true;
							// if ((config.wifi_mode == WIFI_STA_FIX) || (config.wifi_mode == WIFI_AP_STA_FIX))
							//	config.wifi_mode = WIFI_AP_STA_FIX;
							// else
							//	config.wifi_mode = WIFI_AP_FIX;
						}
					}
				}
				if (server.argName(i) == "wificlient")
				{
					if (server.arg(i) != "")
					{
						if (String(server.arg(i)) == "OK")
						{
							wifiSTA = true;
							// if ((config.wifi_mode == WIFI_AP_FIX)||(config.wifi_mode == WIFI_AP_STA_FIX))
							//	config.wifi_mode = WIFI_AP_STA_FIX;
							// else
							//	config.wifi_mode = WIFI_STA_FIX;
						}
					}
				}


				if (server.argName(i) == "wifi_ssidAP")
				{
					if (server.arg(i) != "")
					{
						strcpy(config.wifi_ap_ssid, server.arg(i).c_str());
					}
				}
				if (server.argName(i) == "wifi_passAP")
				{
					if (server.arg(i) != "")
					{
						strcpy(config.wifi_ap_pass, server.arg(i).c_str());
					}
				}
				if (server.argName(i) == "wifi_ssid")
				{
					if (server.arg(i) != "")
					{
						strcpy(config.wifi_ssid, server.arg(i).c_str());
					}
				}
				if (server.argName(i) == "wifi_pass")
				{
					if (server.arg(i) != "")
					{
						strcpy(config.wifi_pass, server.arg(i).c_str());
					}
				}
				if (server.argName(i) == "wifi_protocol")
				{
					if (server.arg(i) != "")
					{
						if (isValidNumber(server.arg(i)))
							config.wifi_protocol = server.arg(i).toInt();
					}
				}
			}
			if (wifiAP && wifiSTA)
			{
				config.wifi_mode = WIFI_AP_STA_FIX;
			}
			else if (wifiAP)
			{
				config.wifi_mode = WIFI_AP_FIX;
			}
			else if (wifiSTA)
			{
				config.wifi_mode = WIFI_STA_FIX;
			}
			else
			{
				config.wifi_mode = WIFI_OFF_FIX;
			}
			saveEEPROM();
			WiFi.disconnect();
			// #ifndef I2S_INTERNAL
			// 			if (adcIsr)
			// 				timerAlarmEnable(timer);
			// #endif
		}
	}

	setHTML();
	webString += "<div class=\"col-xs-10\">\n";
	webString += "<form accept-charset=\"UTF-8\" action=\"/config\" class=\"form-horizontal\" id=\"setting_form\" method=\"post\">\n";

	webString += "<div class = \"col-pad\">\n<h3>WiFi Network</h3>\n";
	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-4 col-xs-12 control-label\">WiFi AP Enable</label>\n";
	String wifiFlageAP = "";
	if ((config.wifi_mode == WIFI_AP_STA_FIX) || (config.wifi_mode == WIFI_AP_FIX))
		wifiFlageAP = "checked";
	webString += "<div class=\"col-sm-4 col-xs-6\"><input class=\"field_checkbox\" id=\"field_checkbox_0\" name=\"wifiAP\" type=\"checkbox\" value=\"OK\" " + wifiFlageAP + "/></div>\n";
	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-4 col-xs-12 control-label\">WiFi AP SSID</label>\n";
	webString += "<div class=\"col-sm-4 col-xs-6\"><input class=\"form-control\" id=\"wifi_ssidAP\" name=\"wifi_ssidAP\" type=\"text\" value=\"" + String(config.wifi_ap_ssid) + "\" /></div>\n";
	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-4 col-xs-12 control-label\">WiFi AP PASSWORD</label>\n";
	webString += "<div class=\"col-sm-4 col-xs-6\"><input class=\"form-control\" id=\"wifi_passAP\" name=\"wifi_passAP\" type=\"password\" value=\"" + String(config.wifi_ap_pass) + "\" /></div>\n";
	webString += "</div><hr width=\"50%\">\n";

	webString += "<div class=\"form-group\">\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-4 col-xs-12 control-label\">WiFi Client SSID</label>\n";
	webString += "<div class=\"col-sm-4 col-xs-6\"><input class=\"form-control\" id=\"wifi_ssid\" name=\"wifi_ssid\" type=\"text\" value=\"" + String(config.wifi_ssid) + "\" /></div>\n";
	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-4 col-xs-12 control-label\">WiFi Client PASSWORD</label>\n";
	webString += "<div class=\"col-sm-4 col-xs-6\"><input class=\"form-control\" id=\"wifi_pass\" name=\"wifi_pass\" type=\"password\" value=\"" + String(config.wifi_pass) + "\" /></div>\n";
	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-4 col-xs-12 control-label\">WiFi Mode</label>\n";
	webString += "<div class=\"col-sm-4 col-xs-6\"><select name=\"wifi_protocol\" id=\"wifi_protocol\">\n";

	if (config.wifi_protocol == 7)
	{
		webString += "<option value=\"1\">IEEE 802.11b</option>\n";
		webString += "<option value=\"3\">IEEE 802.11bg</option>\n";
		webString += "<option value=\"7\" selected>IEEE 802.11bgn</option>\n";
	}
	else if (config.wifi_protocol == 3)
	{
		webString += "<option value=\"1\">IEEE 802.11b</option>\n";
		webString += "<option value=\"3\" selected>IEEE 802.11bg</option>\n";
		webString += "<option value=\"7\">IEEE 802.11bgn</option>\n";
	}
	else if (config.wifi_protocol == 1)
	{
		webString += "<option value=\"1\" selected>IEEE 802.11b</option>\n";
		webString += "<option value=\"3\">IEEE 802.11bg</option>\n";
		webString += "<option value=\"7\">IEEE 802.11bgn</option>\n";
	}
	else
	{
		config.wifi_protocol = 7;
		webString += "<option value=\"1\" selected>IEEE 802.11b</option>\n";
		webString += "<option value=\"3\">IEEE 802.11bg</option>\n";
		webString += "<option value=\"7\">IEEE 802.11bgn</option>\n";
	}
	webString += "</select></div>\n";
	webString += "</div>\n";

	webString += "</div>\n<hr>\n"; // div network

	webString += "<div class = \"col-pad\">\n<h3>WiFi Status</h3>\n";
	webString += "<div class=\"form-group\">\n";
	webString += "<table border=\"0\" cellspacing=\"1\" cellpadding=\"2\">\n";
	webString += "<tr><td align=\"right\"><b>Mode:</b></td>\n";
	webString += "<td align=\"left\">";

	if (config.wifi_mode == WIFI_AP_FIX)
	{
		webString += "AP";
	}
	else if (config.wifi_mode == WIFI_STA_FIX)
	{
		webString += "STA";
	}
	else if (config.wifi_mode == WIFI_AP_STA_FIX)
	{
		webString += "AP+STA";
	}
	else
	{
		webString += "OFF";
	}
	webString += "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>MAC:</b></td>\n";
	webString += "<td align=\"left\">" + String(WiFi.macAddress()) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Channel:</b></td>\n";
	webString += "<td align=\"left\">" + String(WiFi.channel()) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>SSID:</b></td>\n";
	webString += "<td align=\"left\">" + String(WiFi.SSID()) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Local IP:</b></td>\n";
	webString += "<td align=\"left\">" + WiFi.localIP().toString() + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Gateway IP:</b></td>\n";
	webString += "<td align=\"left\">" + WiFi.gatewayIP().toString() + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>DNS:</b></td>\n";
	webString += "<td align=\"left\">" + WiFi.dnsIP().toString() + "</td></tr>\n";
	webString += "</table></div>\n";

	webString += "</div><hr>\n";

	webString += "</div>\n<hr>\n"; // div loger

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-4 col-xs-12 control-label\"></label>\n";
	webString += "<div class=\"col-sm-2 col-xs-4\"><input class=\"btn btn-primary\" id=\"setting_form_sumbit\" name=\"commit\" type=\"submit\" value=\"Save Config\" maxlength=\"80\"/></div>\n";
	webString += "</form><form action=\"/default\" class=\"button_to\" method=\"post\">\n";
	webString += "<div class=\"col-sm-2 col-xs-4\"><input class=\"btn btn-danger\" id=\"default_form_sumbit\" name=\"commit\" type=\"submit\" value=\"Default Config\" maxlength=\"80\"/></div>\n";
	webString += "</form>\n";
	webString += "</div>\n";

	webString += "</div>\n";

	webString += "</body></html>\n";
	server.send(200, "text/html", webString); // send to someones browser when asked
}


void handle_service()
{
	bool noiseEn = false;
	bool agcEn = false;
	bool dtmfEn = false;
	if (server.hasArg("commit"))
	{
		// #ifndef I2S_INTERNAL
		// 		bool adcIsr = timerAlarmEnabled(timer);
		// 		if (adcIsr)
		// 			timerAlarmDisable(timer);
		// #endif
		disconnect_from_host(); // Restart M17 to Reflector
		for (uint8_t i = 0; i < server.args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(server.argName(i));
			// Serial.print("=");
			// Serial.println(server.arg(i));

			if (server.argName(i) == "noiseCheck")
			{
				if (server.arg(i) != "")
				{
					if (String(server.arg(i)) == "OK")
					{
						noiseEn = true;
					}
				}
			}
			if (server.argName(i) == "agcCheck")
			{
				if (server.arg(i) != "")
				{
					if (String(server.arg(i)) == "OK")
					{
						agcEn = true;
					}
				}
			}
	
			if (server.argName(i) == "m17Name")
			{
				if (server.arg(i) != "")
				{
					strcpy(config.reflector_name, server.arg(i).c_str());
				}
			}
			if (server.argName(i) == "m17Host")
			{
				if (server.arg(i) != "")
				{
					strcpy(config.reflector_host, server.arg(i).c_str());
				}
			}
			if (server.argName(i) == "m17Port")
			{
				if (server.arg(i) != "")
				{
					if (isValidNumber(server.arg(i)))
						config.reflector_port = server.arg(i).toInt();
				}
			}

			if (server.argName(i) == "m17Module")
			{
				if (server.arg(i) != "")
				{
					config.reflector_module = server.arg(i).charAt(0);
					current_module = config.reflector_module;
				}
			}

			if (server.argName(i) == "myCallM17")
			{
				if (server.arg(i) != "")
				{
					strcpy(config.mycall, server.arg(i).c_str());
				}
			}

			if (server.argName(i) == "myModM17")
			{
				if (server.arg(i) != "")
				{
					config.mymodule = server.arg(i).charAt(0);
				}
			}



			if (server.argName(i) == "mic_level")
			{
				if (server.arg(i) != "")
				{
					if (isValidNumber(server.arg(i)))
						config.mic = server.arg(i).toInt();
				}
			}

			

			if (server.argName(i) == "codec2_mode")
			{
				if (server.arg(i) != "")
				{
					if (isValidNumber(server.arg(i)))
					{
						config.codec2_mode = server.arg(i).toInt();
						if (config.codec2_mode != 0)
							config.codec2_mode = 2; // CODEC2_MODE_1600 1600bps
					}
				}
			}
		}
		config.noise = noiseEn;
		config.agc = agcEn;
		saveEEPROM();
		// #ifndef I2S_INTERNAL
		// 		if (adcIsr)
		// 			timerAlarmDisable(timer);
		// #endif
	}

	setHTML();
	webString += "<script type=\"text/javascript\">\n";
	webString += "function showVoxValue(newValue){\n";
	webString += "document.getElementById(\"voxShow\").innerHTML=newValue;}\n";
	webString += "function showMicValue(newValue){\n";
	webString += "document.getElementById(\"micShow\").innerHTML=newValue;}\n";
	webString += "</script>\n";
	webString += "<div class=\"col-xs-10\">\n";
	webString += "<form accept-charset=\"UTF-8\" action=\"/service\" class=\"form-horizontal\" id=\"setting_form\" method=\"post\">\n";

	webString += "<div>\n<h3>Digital Voice</h3>\n";
	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-3 col-xs-12 control-label\">M17 Host Name</label>\n";
	webString += "<div class=\"col-sm-2 col-xs-4\"><input class=\"form-control\" id=\"m17Name\" name=\"m17Name\" type=\"text\" value=\"" + String(config.reflector_name) + "\" /></div>\n";
	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-3 col-xs-12 control-label\">M17 Host</label>\n";
	webString += "<div class=\"col-sm-6 col-xs-8\"><input class=\"form-control\" id=\"m17Host\" name=\"m17Host\" type=\"text\" value=\"" + String(config.reflector_host) + "\" /></div>\n";
	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-3 col-xs-12 control-label\">M17 Port</label>\n";
	webString += "<div class=\"col-sm-2 col-xs-4\"><input class=\"form-control\" id=\"m17Port\" name=\"m17Port\" type=\"text\" value=\"" + String(config.reflector_port) + "\" /></div>\n";
	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-3 col-xs-12 control-label\">M17 Module/ROOM</label>\n";
	// webString += "<div class=\"col-sm-1 col-xs-2\"><input class=\"form-control\" id=\"m17Module\" name=\"m17Module\" type=\"text\" value=\"" + String(config.reflector_module) + "\" /></div>\n";
	webString += "<div class=\"col-sm-2 col-xs-6\"><select name=\"m17Module\" id=\"m17Module\">\n";
	for (char mod = 'A'; mod <= 'Z'; mod++)
	{
		if (config.reflector_module == mod)
		{
			webString += "<option value=\"" + String(mod) + "\" selected>" + String(mod) + "</option>\n";
		}
		else
		{
			webString += "<option value=\"" + String(mod) + "\">" + String(mod) + "</option>\n";
		}
	}
	webString += "</select></div>\n";
	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-3 col-xs-12 control-label\">myCallSign</label>\n";
	webString += "<div class=\"col-sm-2 col-xs-4\"><input class=\"form-control\" id=\"myCallM17\" name=\"myCallM17\" type=\"text\" value=\"" + String(config.mycall) + "\" /></div>\n";
	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-3 col-xs-12 control-label\">myModule/SSID</label>\n";
	// webString += "<div class=\"col-sm-1 col-xs-2\"><input class=\"form-control\" id=\"myModM17\" name=\"myModM17\" type=\"text\" value=\"" + String(config.mymodule) + "\" /></div>\n";
	webString += "<div class=\"col-sm-2 col-xs-6\"><select name=\"myModM17\" id=\"myModM17\">\n";
	for (char mod = 'A'; mod <= 'Z'; mod++)
	{
		if (config.mymodule == mod)
		{
			webString += "<option value=\"" + String(mod) + "\" selected>" + String(mod) + "</option>\n";
		}
		else
		{
			webString += "<option value=\"" + String(mod) + "\">" + String(mod) + "</option>\n";
		}
	}
	webString += "</select></div>\n";
	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-3 col-xs-12 control-label\">Audio->M17 Gain</label>\n";
	webString += "<div class=\"col-sm-2 col-xs-6\"><input class=\"form-control\" id=\"mic_level\" name=\"mic_level\" type=\"range\" min=\"1\" max=\"20\" value=\"" + String(config.mic) + "\" onchange=\"showMicValue(this.value)\" /><span id=\"micShow\">" + String(config.mic) + "</span></div>\n";
	webString += "</div>\n";

	String cmSel3200 = "";
	String cmSel1600 = "";
	if (config.codec2_mode == 0)
	{
		cmSel3200 = "selected";
	}
	else
	{
		cmSel1600 = "selected";
	}
	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-3 col-xs-12 control-label\">Codec2 Mode</label>\n";
	webString += "<div class=\"col-sm-2 col-xs-6\"><select name=\"codec2_mode\" id=\"codec2_mode\">\n<option value=\"0\" " + cmSel3200 + ">CODEC2_3200</option>\n<option value=\"2\" " + cmSel1600 + ">CODEC2_1600</option></select></div>\n";
	webString += "</div>\n";

	//#ifdef I2S_INTERNAL
	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-3 col-xs-12 control-label\">Noise Cancel</label>\n";
	String noiseFlage = "";
	if (config.noise)
		noiseFlage = "checked";
	webString += "<div class=\"col-sm-2 col-xs-6\"><input class=\"field_checkbox\" id=\"field_checkbox_1\" name=\"noiseCheck\" type=\"checkbox\" value=\"OK\" " + noiseFlage + "/></div>\n";
	webString += "</div>\n";
	//#endif

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-3 col-xs-12 control-label\">AGC</label>\n";
	String agcFlage = "";
	if (config.agc)
		agcFlage = "checked";
	webString += "<div class=\"col-sm-2 col-xs-6\"><input class=\"field_checkbox\" id=\"field_checkbox_2\" name=\"agcCheck\" type=\"checkbox\" value=\"OK\" " + agcFlage + "/></div>\n";
	webString += "</div>\n";

	webString += "</div>\n";

	webString += "<div class=\"form-group\">\n";
	webString += "<label class=\"col-sm-4 col-xs-12 control-label\"></label>\n";
	webString += "<div class=\"col-sm-2 col-xs-4\"><input class=\"btn btn-primary\" id=\"setting_form_sumbit\" name=\"commit\" type=\"submit\" value=\"Save Config\" maxlength=\"80\"/></div>\n";
	webString += "</div>\n";

	webString += "</form></div>\n";

	webString += "</body></html>\n";
	server.send(200, "text/html", webString); // send to someones browser when asked

	// delay(100);
}



void handle_default()
{
	defaultSetting = true;
	defaultConfig();
	// webMessage = "";
	handle_setting();
	defaultSetting = false;
}

void webService()
{
	server.close();
	// web client handlers
	server.on("/settings", handle_setting);
	server.on("/default", handle_default);
	server.on("/", handle_service);
	server.begin();
}