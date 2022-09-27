#ifndef _WiFiPage_
#define _WiFiPage_
const char title[] PROGMEM = R"KEWL(<!DOCTYPE html
    PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>配网</title>
</head>

<body>
    <div class="main">
        <form name='input' action='/' method='POST'>
            <div class='bb' style='text-align:center;'>
                <table style='width:100%' ;>
                <tr>
                        <td colspan='2' class='ssid' style="font-size:40px;font-weight: bold">风扇配网</td>
                    </tr>
                    <tr>
                        <td colspan='2' class='ssid'></td>
                    </tr>)KEWL";
const char input[] PROGMEM = R"KEWL(<tr>
                        <td colspan='2' class='ssid'></td>
                    </tr>
                    <tr>
                        <td colspan='2' class='bb'>WiFi名称: </td>
                    </tr>
                    <tr>
                        <td colspan='2' class='ssid'><input class='input' type='text' value=''
                                placeholder='输入WiFi名称' name='ssid' id="ssid"></td>
                    </tr>
                    <tr>
                        <td colspan='2' class='bb'>WiFi密码: </td>
                    </tr>
                    <tr>
                        <td colspan='2' class='ssid'><input class='input' type='password' value=''
                                placeholder='输入WiFi密码' name='password' id="pwd"></td>
                    </tr>
                    <tr>
                        <td colspan='2' class='bb'>点灯密钥: </td>
                    </tr>
                    <tr>
                        <td colspan='2' class='ssid'><input class='input' type='text' value=''
                                placeholder='输入点灯密钥' name='authkey'></td>
                    </tr>
                    <tr>
                        <td colspan='2' class='ssid'></td>
                    </tr>
                    <tr>
                        <td colspan='2' class='ssid'><input style='padding-left: 0px; font-weight: bold;' class='input'
                                type='submit' value='点击配网'>
                        </td>
                    </tr>
                </table>
        </form>
    </div>
</body>
<style>
    input,
    div,
    table {
        width: 300px;
        margin: auto;
    }

    .ssidList {
        width: 100%;
        height: 100%;
        cursor: pointer;
        text-align: left;
    }

    .ssid {
        text-align: center;
    }

    td {
        height: 25px;
        text-align: left;
    }

    .main {
        height: 100%;
        font-weight: bold;
    }

    .input {
        resize: none;
        outline: none;
        height: 45px;
        border: 1px solid #d0d1ce;
        font-size: 14px;
        color: #000;
        border-radius: 10px;
        border: 1px solid #dcdfe6;
        background-color: #ffffff;
        text-align:center;
    }

    .bb {
        text-align: left;
        font-weight: bold;
    }
</style>
<script>
    var ssidText = document.getElementById("ssid");
    var pwdText = document.getElementById("pwd");
    function c(ssid) {
        ssidText.value = ssid;
        pwdText.scrollIntoView();
        pwdText.value = "";
        pwdText.focus();
    }
</script>

</html>)KEWL";
#endif