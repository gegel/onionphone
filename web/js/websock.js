  var secureCb;
  var secureCbLabel;
  var wsUri;
  var consoleLog;
  var connectBut;
  var sendMessage;
  var sendBut;
  var clearLogBut;
  var helpCb;
  var connected = 0;
  var codecSel;
  var gainCb;
  var noiseCb;
  var vocCb;
  var testCb;
  var jitterSel;
  
  function runAcc(e) {
    if (!e || e==-1 || e.which == 13 || e.keyCode == 13) {
        if(!connected) 
		{
			logToConsole("No connection!");
			return false;	
		}
	    var ss  = document.getElementById("setacc");
		if(e==-1) ss.value="";
		if(ss.value[0]!='-') ss.value="-Y"+ss.value;
		logToConsole("Setting access to secret key...");
        websocket.send(ss.value);
		ss.value="";
        return false;
    }
}
  
 function runPas(e) {
    if (!e || e==-1 || e.which == 13 || e.keyCode == 13) {
        if(!connected) 
		{
			logToConsole("No connection!");
			return false;	
		}
	    var ss  = document.getElementById("setpas");
		if(e==-1) 
		{
			if(ss.value!="") ss.value="Please enter our password (for '" + ss.value +"')!";
			else ss.value="Please enter our password (for me)!";
			logToConsole("Sending request for indentification...");
		}
		else
		{
			if(ss.value[0]!='-') ss.value="-P"+ss.value;
			logToConsole("Setting shared password...");
		}
        websocket.send(ss.value);
		ss.value="";
        return false;
    }
}  
  
  function runKey(e) {
    if (!e || e==-1 || e.which == 13 || e.keyCode == 13) {
        if(!connected) 
		{
			logToConsole("No connection!");
			return false;	
		}
	    var ss  = document.getElementById("setkey");
		if(e==-1) ss.value="";
		if(ss.value[0]!='-') ss.value="-K"+ss.value;
		logToConsole("Sending public key: "+ss.value);
        websocket.send(ss.value);
		ss.value="";
        return false;
    }
}
  
  function runScript(e) {
    if (e.which == 13 || e.keyCode == 13) {
        if(!connected) 
		{
			logToConsole("No connection!");
			return false;	
		}
	
		logToConsole(sendMessage.value);
        websocket.send(sendMessage.value);
		sendMessage.value="";
        return false;
    }
}

  function echoHandlePageLoad()
  {
    if (window.WebSocket)
    {
      document.getElementById("webSocketSupp").style.display = "block";
    }
    else
    {
      document.getElementById("noWebSocketSupp").style.display = "block";
    }

    secureCb = document.getElementById("secureCb");
    secureCb.checked = false;
    secureCb.onclick = toggleTls;
    
    secureCbLabel = document.getElementById("secureCbLabel")
	
	helpCb = document.getElementById("helpCb");
    helpCb.checked = true;
	
    wsUri = document.getElementById("wsUri");
    toggleTls();
    
    connectBut = document.getElementById("connect");
    connectBut.onclick = doConnect;
    
    sendMessage = document.getElementById("sendMessage");
	sendMessage.ondblclick = doMsg;

    sendBut = document.getElementById("send");
    sendBut.onclick = doSend;


	udpBut = document.getElementById("budp");
    udpBut.onclick = doUdp;
	
	upBut = document.getElementById("bup");
    upBut.onclick = doUp;
	
	tcpBut = document.getElementById("btcp");
    tcpBut.onclick = doTcp;
	
	callBut = document.getElementById("bcall");
    callBut.onclick = doCall;
	
	leftBut = document.getElementById("bleft");
    leftBut.onclick = doLeft;
	
	rightBut = document.getElementById("bright");
    rightBut.onclick = doRight;
	
	talkBut = document.getElementById("btalk");
	talkBut.onmousedown = doSpeek;
	talkBut.onmouseup = doMute;
	
	exitBut = document.getElementById("bexit");
    exitBut.onclick = doExit;
	
	downBut = document.getElementById("bdown");
    downBut.onclick = doDown;
	
	vadBut = document.getElementById("bvad");
    vadBut.onclick = doVad;
	
	brkBut = document.getElementById("bbrk");
    brkBut.onclick = doBrk;
	
	haltBut = document.getElementById("bhalt");
    haltBut.ondblclick = doHalt;
	
	ansBut = document.getElementById("bans");
    ansBut.onclick = doAns;
	
	pingBut = document.getElementById("bping");
    pingBut.onclick = doPing;
	
	findBut = document.getElementById("bfind");
    findBut.onclick = doFind;
	
	nameBut = document.getElementById("bname");
    nameBut.onclick = doName;
	
	bookBut = document.getElementById("bbook");
    bookBut.onclick = doBook;
	
	autBut = document.getElementById("baut");
    autBut.onclick = doAut;
	
	otherBut = document.getElementById("bother");
    otherBut.onclick = doOther;
	
	
	//accBut =  document.getElementById("bacc");
	//pasSet = document.getElementById("setpas");
	//pasBut = document.getElementById("bpas");
	//keySet = document.getElementById("setkey");
	//keyBut = document.getElementById("bkey");

    consoleLog = document.getElementById("consoleLog");

    clearLogBut = document.getElementById("clearLogBut");
    clearLogBut.onclick = clearLog;

    var nojavaDiv=document.getElementById("nojava");
	nojavaDiv.style.display="none";

    setGuiConnected(false);
	doBook();
	doAut();
	doOther();
	
    document.getElementById("send").onclick = doSend;
	
	codecSel = document.getElementById("codec");
	codecSel.onchange = setCodec;
	
	noiseCb = document.getElementById("noiseCb");
	noiseCb.onclick = setNoise;
	
	vocCb = document.getElementById("vocCb");
	vocCb.onclick = setVoc;
	
	testCb = document.getElementById("testCb");
	testCb.onclick = setTest;
	
	gainCb = document.getElementById("gainCb");
	gainCb.onclick = setGain;
	
	jitterSel = document.getElementById("jitter");
	jitterSel.onchange = setJitter;

  }

  function toggleTls()
  {
    var wsPort = (window.location.port.toString() === "" ? "" : ":"+window.location.port)
    if (wsUri.value === "") {
        wsUri.value = "ws://localhost:8000" + window.location.hostname.replace("www", "echo") + wsPort;
    }
    
    if (secureCb.checked)
    {
      wsUri.value = wsUri.value.replace("ws:", "wss:");
    }
    else
    {
      wsUri.value = wsUri.value.replace ("wss:", "ws:");
    }
  }
  
  function doConnect()
  {
    if(connected)
	{
		websocket.close();
		return;
	}
	
	if (window.MozWebSocket)
    {
        logToConsole('<span style="color: red;"><strong>Info:</strong> This browser supports WebSocket using the MozWebSocket constructor</span>');
        window.WebSocket = window.MozWebSocket;
    }
    else if (!window.WebSocket)
    {
        logToConsole('<span style="color: red;"><strong>Error:</strong> This browser does not have support for WebSocket</span>');
        return;
    }

    // prefer text messages
    var uri = wsUri.value;
    if (uri.indexOf("?") == -1) {
        uri += "?encoding=text";
    } else {
        uri += "&encoding=text";
    }
    websocket = new WebSocket(uri);
    websocket.onopen = function(evt) { onOpen(evt) };
    websocket.onclose = function(evt) { onClose(evt) };
    websocket.onmessage = function(evt) { onMessage(evt) };
    websocket.onerror = function(evt) { onError(evt) };
  }
  
  function doSend()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	if(sendMessage.value!="")
	{
		logToConsole(sendMessage.value);
    	websocket.send(sendMessage.value);
		sendMessage.value="";
	}
	else
	{
	 logToConsole("Sending OK");
     websocket.send("#10");	
	}
  }
  
  function doUdp()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Trying UDP...");
    websocket.send("-S");
  }
  
  function doUp()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	websocket.send("#4");
  }
  
  function doTcp()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Back to Onion...");
    websocket.send("-O");
  }
  
  function doCall()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	if(sendMessage.value[0] != '-') sendMessage.value = "-O" + sendMessage.value;
	else
	{
		logToConsole("Call: " + sendMessage.value);
    	websocket.send(sendMessage.value);
		sendMessage.value="";
	}
  }
  
  function doLeft()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	websocket.send("#7");
  }
  
  function doRight()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	websocket.send("#6");
  }
  
  function doSpeek()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	//logToConsole("Speek");
	websocket.send("-R1");
  }
  
  function doMute()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	//logToConsole("Mute");
	websocket.send("-R0");
  }

 function doExit()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("OnionPhone status:");
    websocket.send("-RI");
  }
  
  function doDown()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	websocket.send("#5");
  }
  
  function doVad()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Enabling VAD");
    websocket.send("#3");
  }
  
  function doBrk()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Termination of the call...");
	websocket.send("-H");
  }
  
  
  
  
  function doAns()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Accepts the call...");
	websocket.send("#10");
  }
  
  function doPing()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Ping...");
	websocket.send("-RL");
  }
  
  function doFind()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Searching in Address Book: "+sendMessage.value);
	sendMessage.value = "-V" + sendMessage.value;
	websocket.send(sendMessage.value);
	sendMessage.value="";
  }
  
  function doName()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Extracting the name..."+sendMessage.value);
	sendMessage.value = "-E" + sendMessage.value;
	websocket.send(sendMessage.value);
	sendMessage.value="";
  }
  
  
  
  
  
  function doMsg()
  {
    if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole(sendMessage.value);
    websocket.send(sendMessage.value);
	sendMessage.value="";
  }
   
  function doBook()
  {	
	bl = document.getElementById("book");
	if(bookBut.innerHTML==="+")
	{
	 	bookBut.innerHTML="-";
	 	bl.style.display="block";
	}
	else 
	{	
		bookBut.innerHTML="+";
	    bl.style.display="none";
	}
  }
  
  function doAut()
  {
    bl = document.getElementById("aut");
	if(autBut.innerHTML==="+")
	{
	 	autBut.innerHTML="-";
	 	bl.style.display="block";
	}
	else 
	{	
		autBut.innerHTML="+";
	    bl.style.display="none";
	}
  }
  
   function doOther()
  {
    bl = document.getElementById("other");
	if(otherBut.innerHTML==="+")
	{
	 	otherBut.innerHTML="-";
	 	bl.style.display="block";
	}
	else 
	{	
		otherBut.innerHTML="+";
	    bl.style.display="none";
	}
  }
  
  function setCodec()
  {
	if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Changing audio Codec...");
    websocket.send(codecSel.value);  
  }
  
  function doHalt()
  {
	if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Shutdown OnionPhone...");
    websocket.send("-X");
  }
  
  function setGain()
  {
    var cm;
	if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	if(gainCb.checked) cm="-Q1"; else cm="-Q-1";
	logToConsole("Changing AGC...");
	websocket.send(cm);   
  }

   function setNoise()
  {
    var cm;
	if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	if(noiseCb.checked) cm="-Q2"; else cm="-Q-2";
	logToConsole("Changing NPP...");
	websocket.send(cm);
  }
  
   function setVoc()
  {
    var cm;
	if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	if(vocCb.checked) cm="-Q3"; else cm="-Q-3";
	logToConsole("Changing VOC...");
	websocket.send(cm);   
  }
  
   function setTest()
  {
    var cm;
	if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	if(testCb.checked) cm="-RV"; else cm="-R";
	logToConsole("Changing Test_mode...");
	websocket.send(cm);   
  }
  
  function setJitter()
  {
	if(!connected) 
	{
		logToConsole("No connection!");
		return;	
	}
	
	logToConsole("Setting Jitter compensation...");
    websocket.send(jitterSel.value);  
  }

  function logToConsole(message)
  {
    var pre = document.createElement("p");
    pre.style.wordWrap = "break-word";
    pre.innerHTML = getSecureTag()+message;
    consoleLog.appendChild(pre);

    while (consoleLog.childNodes.length > 50)
    {
      consoleLog.removeChild(consoleLog.firstChild);
    }
    
    consoleLog.scrollTop = consoleLog.scrollHeight;
  }
  
  function onOpen(evt)
  {
    logToConsole("CONNECTED");
    setGuiConnected(true);
  }
  
  function onClose(evt)
  {
    logToConsole("DISCONNECTED");
    setGuiConnected(false);
  }
  
  function onMessage(evt)
  {
    if(evt.data.charAt(0) == '-' || evt.data.charAt(0) == '*')
	sendMessage.value=evt.data;
	else if(evt.data.charAt(0) == '!') 
	logToConsole('<span style="color: red;">' + evt.data+'</span>');
	else if(evt.data.charAt(0) == '>') 
	logToConsole('<span style="color: green;">' + evt.data+'</span>');
	else logToConsole('<span style="color: blue;">' + evt.data+'</span>');
  }

  function onError(evt)
  {
    logToConsole('<span style="color: red;">ERROR:</span> ' + evt.data);
  }
  
  function setGuiConnected(isConnected)
  {
    if(isConnected)
	{
		connectBut.innerHTML="Disconnect";
		connectBut.style.color="green";
		connected=1;
	}
	else
	{
		connectBut.innerHTML="Connect";
		connectBut.style.color="red";
		connected=0;	
	}
	
	/*
	wsUri.disabled = isConnected;
    connectBut.disabled = isConnected;
    //disconnectBut.disabled = !isConnected;
    sendMessage.disabled = !isConnected;
    sendBut.disabled = !isConnected;
	
	secureCbdisabled = 0;
	udpBut.disabled = !isConnected;
	upBut.disabled = !isConnected;
	tcpBut.disabled = !isConnected;
	callBut.disabled = !isConnected;
	leftBut.disabled = !isConnected;
	rightBut.disabled = !isConnected;
	talkBut.disabled = !isConnected;
	exitBut.disabled = !isConnected;
	downBut.disabled = !isConnected;
	vadBut.disabled = !isConnected;
	brkBut.disabled = !isConnected;
	
    secureCb.disabled = isConnected;
    var labelColor = "black";
    if (isConnected)
    {
      labelColor = "#999999";
    }
     secureCbLabel.style.color = labelColor;
    */
  }
	
	function clearLog()
	{
		while (consoleLog.childNodes.length > 0)
		{
			consoleLog.removeChild(consoleLog.lastChild);
		}
	}
	
	function getSecureTag()
	{
		if (secureCb.checked)
		{
			return '<img src="img/tls-lock.png" width="6px" height="9px"> ';
		}
		else
		{
			return '';
		}
	}
  
  window.addEventListener("load", echoHandlePageLoad, false);
