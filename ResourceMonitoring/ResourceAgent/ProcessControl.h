
//////////////////////////////////////////////////////////////////////////////////////////////////
// Executes the given command using CreateProcess() and WaitForSingleObject().
// Returns FALSE if the command could not be executed or if the exit code could not be determined.
BOOL executeCommandLine(CString cmdLine, DWORD & exitCode)
{
	PROCESS_INFORMATION processInformation = {0};
	STARTUPINFO startupInfo                = {0};
	startupInfo.cb                         = sizeof(startupInfo);
	int nStrBuffer                         = cmdLine.GetLength() + 50;


	// Create the process
	BOOL result = CreateProcess(NULL, cmdLine.GetBuffer(nStrBuffer), 
		NULL, NULL, FALSE, 
		NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 
		NULL, NULL, &startupInfo, &processInformation);
	cmdLine.ReleaseBuffer();


	if (!result)
	{
		// CreateProcess() failed
		// Get the error from the system
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

		// Display the error
		CString strError = (LPTSTR) lpMsgBuf;
		TRACE(_T("::executeCommandLine() failed at CreateProcess()\nCommand=%s\nMessage=%s\n\n"), cmdLine, strError);

		// Free resources created by the system
		LocalFree(lpMsgBuf);

		// We failed.
		return FALSE;
	}
	else
	{
		// Successfully created the process.  Wait for it to finish.
		WaitForSingleObject( processInformation.hProcess, INFINITE );

		// Get the exit code.
		result = GetExitCodeProcess(processInformation.hProcess, &exitCode);

		// Close the handles.
		CloseHandle( processInformation.hProcess );
		CloseHandle( processInformation.hThread );

		if (!result)
		{
			// Could not get exit code.
			TRACE(_T("Executed command but couldn't get exit code.\nCommand=%s\n"), cmdLine);
			return FALSE;
		}


		// We succeeded.
		return TRUE;
	}
}

CString ExecuteExternalFile(CString csExeName, CString csArguments)
{
	CString csExecute;
	csExecute=csExeName + " " + csArguments;

	SECURITY_ATTRIBUTES secattr; 
	ZeroMemory(&secattr,sizeof(secattr));
	secattr.nLength = sizeof(secattr);
	secattr.bInheritHandle = TRUE;

	HANDLE rPipe, wPipe;

	//Create pipes to write and read data
	CreatePipe(&rPipe,&wPipe,&secattr,0);
	//
	STARTUPINFO sInfo; 
	ZeroMemory(&sInfo,sizeof(sInfo));
	PROCESS_INFORMATION pInfo; 
	ZeroMemory(&pInfo,sizeof(pInfo));
	sInfo.cb=sizeof(sInfo);
	sInfo.dwFlags=STARTF_USESTDHANDLES;
	sInfo.hStdInput=NULL; 
	sInfo.hStdOutput=wPipe; 
	sInfo.hStdError=wPipe;
	char command[1024]; strcpy(command,  
		csExecute.GetBuffer(csExecute.GetLength()));

	//Create the process here.
	CreateProcess(0 command,0,0,TRUE,
		NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,0,0,&sInfo,&pInfo);
	CloseHandle(wPipe);

	//now read the output pipe here.
	char buf[100];
	DWORD reDword; 
	CString m_csOutput,csTemp;
	BOOL res;
	do
	{
		res=::ReadFile(rPipe,buf,100,&reDword,0);
		csTemp=buf;
		m_csOutput+=csTemp.Left(reDword);
	}while(res);
	return m_csOutput;
}


DWORD RunSilent(char* strFunct, char* strstrParams)
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	char Args[4096];
	char *pEnvCMD = NULL;
	char *pDefaultCMD = "CMD.EXE";
	ULONG rc;

	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_HIDE;

	Args[0] = 0;

	pEnvCMD = getenv("COMSPEC");

	if(pEnvCMD){

		strcpy(Args, pEnvCMD);
	}
	else{
		strcpy(Args, pDefaultCMD);
	}

	// "/c" option - Do the command then terminate the command window
	strcat(Args, " /c "); 
	//the application you would like to run from the command window
	strcat(Args, strFunct);  
	strcat(Args, " "); 
	//the parameters passed to the application being run from the command window.
	strcat(Args, strstrParams); 

	if (!CreateProcess( NULL, Args, NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE, 
		NULL, 
		NULL,
		&StartupInfo,
		&ProcessInfo))
	{
		return GetLastError();		
	}

	WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
	if(!GetExitCodeProcess(ProcessInfo.hProcess, &rc))
		rc = 0;

	CloseHandle(ProcessInfo.hThread);
	CloseHandle(ProcessInfo.hProcess);

	return rc;

}
