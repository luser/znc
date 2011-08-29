/*
 * Copyright (C) 2004-2011  See the AUTHORS file for details.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include "Chan.h"
#include "HTTPSock.h"
#include "Server.h"
#include "Template.h"
#include "User.h"
#include "znc.h"
#include "WebModules.h"
#include <map>
#include <sstream>
#include <vector>

using std::map;
using std::stringstream;
using std::vector;

class CWebClientMod;

class CEventSock : public CSocket {
public:
  CEventSock(CWebClientMod* pModule);
  ~CEventSock() { Cleanup(); }
  virtual void SockError(int iErrno) { Cleanup(); }

private:
  void Cleanup();

  CWebClientMod* m_pModule;
};

class CWebClientMod : public CModule {
private:
  typedef map<CString,CString> EventMap;
  vector<CEventSock*> m_sockets;

  void SendEvent(const CString& event, const EventMap& data) {
    if (m_sockets.size() == 0)
      return;

    stringstream s;
    s << "event: " << event << "\n"
      << "data: {";
    // Horrible JSON serialization;
    for (EventMap::iterator it = data.begin();
         it != data.end();
         it++) {
      s << "\"" << it->first << "\"";
    }
    s "}\n\n";
    for (unsigned int i=0; i<m_sockets.size(); i++) {
      m_sockets[i]->Write(s.str());
    }
  }

public:
	MODCONSTRUCTOR(CWebClientMod) {
		AddHelpCommand();
		//AddCommand("List",   static_cast<CModCommand::ModCmdFunc>(&CWebClientMod::ListCommand));
	}

	virtual ~CWebClientMod() {}

	virtual CString GetWebMenuTitle() { return "WebClient"; }

	virtual bool OnLoad(const CString& sArgs, CString& sMessage) {
		return true;
	}

	virtual EModRet OnModuleUnloading(CModule* pModule, bool& bSuccess, CString& sRetMsg) {
          return CONTINUE;
	}

	virtual bool OnBoot() { return true; }

	virtual void OnIRCConnected() {
	}

	virtual void OnIRCDisconnected() {
	}

	virtual EModRet OnIRCRegistration(CString& sPass, CString& sNick, CString& sIdent, CString& sRealName) {
		return CONTINUE;
	}

	virtual void OnClientLogin() {
	}

	virtual void OnChanPermission(const CNick& OpNick, const CNick& Nick, CChan& Channel, unsigned char uMode, bool bAdded, bool bNoChange) {
        }

	virtual void OnOp(const CNick& OpNick, const CNick& Nick, CChan& Channel, bool bNoChange) {
        }

	virtual void OnDeop(const CNick& OpNick, const CNick& Nick, CChan& Channel, bool bNoChange) {
		PutModule(((bNoChange) ? "[0] [" : "[1] [") + OpNick.GetNick() + "] deopped [" + Nick.GetNick() + "] on [" + Channel.GetName() + "]");
	}

	virtual void OnVoice(const CNick& OpNick, const CNick& Nick, CChan& Channel, bool bNoChange) {
        }

	virtual void OnDevoice(const CNick& OpNick, const CNick& Nick, CChan& Channel, bool bNoChange) {
        }

	virtual void OnKick(const CNick& OpNick, const CString& sKickedNick, CChan& Channel, const CString& sMessage) {
        }

	virtual void OnQuit(const CNick& Nick, const CString& sMessage, const vector<CChan*>& vChans) {
        }

	virtual EModRet OnTimerAutoJoin(CChan& Channel) {
		return CONTINUE;
        }

	virtual void OnJoin(const CNick& Nick, CChan& Channel) {
        }

	virtual void OnPart(const CNick& Nick, CChan& Channel, const CString& sMessage) {
        }

	virtual void OnNick(const CNick& OldNick, const CString& sNewNick, const vector<CChan*>& vChans) {
	}

	virtual void OnRawMode(const CNick& OpNick, CChan& Channel, const CString& sModes, const CString& sArgs) {
        }

	virtual EModRet OnRaw(CString& sLine) {
		return CONTINUE;
        }

	virtual EModRet OnUserRaw(CString& sLine) {
		return CONTINUE;
	}

	virtual EModRet OnUserCTCPReply(CString& sTarget, CString& sMessage) {
		return CONTINUE;
	}

	virtual EModRet OnCTCPReply(CNick& Nick, CString& sMessage) {
		return CONTINUE;
	}

	virtual EModRet OnUserCTCP(CString& sTarget, CString& sMessage) {
		return CONTINUE;
	}

	virtual EModRet OnPrivCTCP(CNick& Nick, CString& sMessage) {
		return CONTINUE;
	}

	virtual EModRet OnChanCTCP(CNick& Nick, CChan& Channel, CString& sMessage) {
		return CONTINUE;
	}

	virtual EModRet OnUserNotice(CString& sTarget, CString& sMessage) {
		return CONTINUE;
	}

	virtual EModRet OnPrivNotice(CNick& Nick, CString& sMessage) {
		return CONTINUE;
	}

	virtual EModRet OnChanNotice(CNick& Nick, CChan& Channel, CString& sMessage) {
		return CONTINUE;
	}

	virtual EModRet OnTopic(CNick& Nick, CChan& Channel, CString& sTopic) {
		return CONTINUE;
	}

	virtual EModRet OnUserTopic(CString& sTarget, CString& sTopic) {
		return CONTINUE;
	}

	virtual EModRet OnUserMsg(CString& sTarget, CString& sMessage) {
		return CONTINUE;
	}

	virtual EModRet OnPrivMsg(CNick& Nick, CString& sMessage) {
		return CONTINUE;
	}

	virtual EModRet OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage) {
          SendEvent("chanmsg", GetUser()->GetIRCServer() + ", " + Nick.GetNickMask() + ", " + Channel.GetName() + ", " + sMessage);
		return CONTINUE;
	}

	virtual void OnModCommand(const CString& sCommand) {
	}

	virtual EModRet OnStatusCommand(CString& sCommand) {
		return CONTINUE;
	}

  void RemoveSocket(CEventSock* sock) {
    for (unsigned int i = 0; i < m_sockets.size(); i++) {
      if (m_sockets[i] == sock) {
        m_sockets.erase(m_sockets.begin() + i);
        break;
      }
    }
  }

	virtual bool OnWebPreRequest(CWebSock& WebSock, const CString& sPageName) {
          if (sPageName != "events")
            return false;

          WebSock.AddHeader("Cache-Control", "no-cache");
          WebSock.PrintHeader(0, "text/event-stream");

          CEventSock* sock = new CEventSock(this);
          CZNC::Get().GetManager().SwapSockByAddr(sock, &WebSock);
          sock->SetSockName("WebClient::Events");
          m_sockets.push_back(sock);
          return true;
	}

	virtual bool OnWebRequest(CWebSock& WebSock, const CString& sPageName, CTemplate& Tmpl) {
		if (sPageName == "index") {
                  //TODO: send existing user info/channel etc
                  CUser* user = GetUser();
                  Tmpl["Nick"] = user->GetCurNick();
                  if (user->IsIRCConnected()) {
                    const vector<CServer*>& servers = user->GetServers();
                    for (unsigned int i = 0; i < servers.size(); i++) {
                      CTemplate& l = Tmpl.AddRow("ServerLoop");
                      l["Server"] = servers[i]->GetName();
                    }
                  }
                  return true;
		}

		return false;
	}
};

CEventSock::CEventSock(CWebClientMod* pModule) : CSocket(pModule),
                                                 m_pModule(pModule) {}

void CEventSock::Cleanup() {
  if (m_pModule) {
    m_pModule->RemoveSocket(this);
    m_pModule = NULL;
  }
}

template<> void TModInfo<CWebClientMod>(CModInfo& Info) {
	Info.SetWikiPage("webclient");
}

MODULEDEFS(CWebClientMod, "Chat via a web browser")
