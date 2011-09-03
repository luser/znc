/*
 * Copyright (C) 2004-2011  See the AUTHORS file for details.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include "Chan.h"
#include "HTTPSock.h"
#include "IRCNetwork.h"
#include "Server.h"
#include "Template.h"
#include "User.h"
#include "znc.h"
#include "WebModules.h"
#include <list>
#include <map>
#include <sstream>
#include <vector>

using std::list;
using std::map;
using std::stringstream;
using std::vector;

class CWebClientMod;

class CEventSock : public CSocket {
public:
  CEventSock(CWebClientMod* pModule);
  ~CEventSock() { Cleanup(); }
  virtual void Disconnected() {
    DEBUG("CEventSock closed");
    Cleanup();
  }

private:
  void Cleanup();

  CWebClientMod* m_pModule;
};

static CString ChanList(const vector<CChan*>& vChans) {
  CString s;
  for (unsigned int i = 0; i < vChans.size(); i++) {
    s += vChans[i]->GetName();
    if (i != vChans.size() - 1)
      s += ",";
  }
  return s;
}

const unsigned int kMaxEventQueue = 50;

class CWebClientMod : public CModule {
private:
  typedef map<CString,CString> EventMap;
  vector<CEventSock*> m_sockets;

  // Save a buffer of events in case a client gets disconnected.
  unsigned int m_nextID;
  typedef list<pair<unsigned int, string> > EventList;
  EventList m_events;

  void SendEvent(const CString& event, const EventMap& data) {
    stringstream s;
    s << "event: " << event << "\n"
      << "id: " << m_nextID << "\n"
      << "data: {";
    // Horrible JSON serialization
    for (EventMap::const_iterator it = data.begin();
         it != data.end();
         it++) {
      s << "\"" << it->first.Replace_n("\"", "\\\"")
        << "\": \""<< it->second.Replace_n("\"", "\\\"") << "\", ";
    }
    s << "\"timestamp\": " << time(NULL);
    s << "}\n\n";

    m_events.push_back(make_pair(m_nextID, s.str()));
    m_nextID++;

    while (m_events.size() > kMaxEventQueue) {
      m_events.pop_front();
    }

    for (unsigned int i=0; i<m_sockets.size(); i++) {
      m_sockets[i]->Write(s.str());
    }
  }

public:
	MODCONSTRUCTOR(CWebClientMod) {
		m_nextID = 0;
		AddHelpCommand();
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

	virtual void OnChanPermission(const CNick& OpNick, const CNick& Nick, CChan& Channel, unsigned char uMode, bool bAdded, bool bNoChange) {
        }

	virtual void OnOp(const CNick& OpNick, const CNick& Nick, CChan& Channel, bool bNoChange) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["op"] = OpNick.GetNick();
          m["user"] = Nick.GetNick();
          m["opped"] = "true";
          m["channel"] = Channel.GetName();
          SendEvent("op", m);
        }

	virtual void OnDeop(const CNick& OpNick, const CNick& Nick, CChan& Channel, bool bNoChange) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["op"] = OpNick.GetNick();
          m["user"] = Nick.GetNick();
          m["opped"] = "false";
          m["channel"] = Channel.GetName();
          SendEvent("op", m);
	}

	virtual void OnVoice(const CNick& OpNick, const CNick& Nick, CChan& Channel, bool bNoChange) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["op"] = OpNick.GetNick();
          m["user"] = Nick.GetNick();
          m["voiced"] = "true";
          m["channel"] = Channel.GetName();
          SendEvent("voice", m);
        }

	virtual void OnDevoice(const CNick& OpNick, const CNick& Nick, CChan& Channel, bool bNoChange) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["op"] = OpNick.GetNick();
          m["user"] = Nick.GetNick();
          m["voiced"] = "false";
          m["channel"] = Channel.GetName();
          SendEvent("voice", m);
        }

	virtual void OnKick(const CNick& OpNick, const CString& sKickedNick, CChan& Channel, const CString& sMessage) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["op"] = OpNick.GetNick();
          m["user"] = sKickedNick;
          m["channel"] = Channel.GetName();
          m["msg"] = sMessage;
          SendEvent("kick", m);
        }

	virtual void OnQuit(const CNick& Nick, const CString& sMessage, const vector<CChan*>& vChans) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["user"] = Nick.GetNick();
          m["mask"] = Nick.GetNickMask();
          m["channels"] = ChanList(vChans);
          m["msg"] = sMessage;
          SendEvent("quit", m);
        }

	virtual EModRet OnTimerAutoJoin(CChan& Channel) {
		return CONTINUE;
        }

	virtual void OnJoin(const CNick& Nick, CChan& Channel) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["user"] = Nick.GetNick();
          m["mask"] = Nick.GetNickMask();
          m["channel"] = Channel.GetName();
          if (Nick.GetNickMask() == m_pNetwork->GetIRCNick().GetNickMask()) {
            m["self"] = "true";
            m["topic"] = Channel.GetTopic();
          }

          SendEvent("join", m);
        }

	virtual void OnPart(const CNick& Nick, CChan& Channel, const CString& sMessage) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["user"] = Nick.GetNick();
          m["mask"] = Nick.GetNickMask();
          m["channel"] = Channel.GetName();
          m["msg"] = sMessage;
          if (Nick.GetNickMask() == m_pNetwork->GetIRCNick().GetNickMask())
            m["self"] = "true";

          SendEvent("part", m);
        }

	virtual void OnNick(const CNick& OldNick, const CString& sNewNick, const vector<CChan*>& vChans) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["old"] = OldNick.GetNick();
          m["new"] = sNewNick;
          m["channels"] = ChanList(vChans);
          SendEvent("nick", m);
	}

	virtual void OnRawMode(const CNick& OpNick, CChan& Channel, const CString& sModes, const CString& sArgs) {
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
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["user"] = Nick.GetNick();
          m["channel"] = Channel.GetName();
          m["topic"] = sTopic;
          SendEvent("topic", m);
          return CONTINUE;
	}

	virtual EModRet OnUserTopic(CString& sTarget, CString& sTopic) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["user"] = m_pNetwork->GetCurNick();
          m["self"] = "true";
          m["channel"] = sTarget;
          m["topic"] = sTopic;
          SendEvent("topic", m);
          return CONTINUE;
	}

	virtual EModRet OnUserMsg(CString& sTarget, CString& sMessage) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["msg"] = sMessage;
          m["self"] = "true";

          if (m_pNetwork->IsChan(sTarget)) {
            m["channel"] = sTarget;
            m["user"] = m_pNetwork->GetCurNick();
            SendEvent("chanmsg", m);
          }
          else {
            m["user"] = sTarget;
            SendEvent("privmsg", m);
          }

          return CONTINUE;
	}

	virtual EModRet OnUserAction(CString& sTarget, CString& sMessage) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["msg"] = sMessage;
          m["self"] = "true";
          m["action"] = "true";

          if (m_pNetwork->IsChan(sTarget)) {
            m["channel"] = sTarget;
            m["user"] = m_pNetwork->GetCurNick();
            SendEvent("chanmsg", m);
          }
          else {
            m["user"] = sTarget;
            SendEvent("privmsg", m);
          }

          return CONTINUE;
	}

	virtual EModRet OnPrivMsg(CNick& Nick, CString& sMessage) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["user"] = Nick.GetNick();
          m["mask"] = Nick.GetNickMask();
          m["msg"] = sMessage;
          SendEvent("privmsg", m);
          return CONTINUE;
	}

	virtual EModRet OnPrivAction(CNick& Nick, CString& sMessage) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["user"] = Nick.GetNick();
          m["mask"] = Nick.GetNickMask();
          m["action"] = "true";
          m["msg"] = sMessage;
          SendEvent("privmsg", m);
          return CONTINUE;
	}

	virtual EModRet OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["user"] = Nick.GetNick();
          m["channel"] = Channel.GetName();
          m["msg"] = sMessage;
          SendEvent("chanmsg", m);
          return CONTINUE;
	}

	virtual EModRet OnChanAction(CNick& Nick, CChan& Channel, CString& sMessage) {
          EventMap m;
          m["network"] = m_pNetwork->GetName();
          m["user"] = Nick.GetNick();
          m["action"] = "true";
          m["channel"] = Channel.GetName();
          m["msg"] = sMessage;
          SendEvent("chanmsg", m);
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

          CString lastIDHeader = WebSock.GetRequestHeader("Last-Event-ID");
          if (!lastIDHeader.empty()) {
            DEBUG("lastID: " << lastIDHeader);
            unsigned int lastID = lastIDHeader.ToUInt();
            // See if we have any buffered events.
            if (!m_events.empty() && m_events.back().first >= lastID) {
              for (EventList::const_iterator it = m_events.begin();
                   it != m_events.end();
                   it++) {
                if (it->first <= lastID)
                  continue;
                DEBUG("Sending queued event " << it->first);
                WebSock.Write(it->second);
              }
            }
            else {
              if (m_events.empty()) {
                DEBUG("Event queue empty");
              }
              else if (m_events.back().first > lastID) {
                DEBUG("No pending events: " << m_events.back().first << " > " << lastID);
              }
            }
          }

          CEventSock* sock = new CEventSock(this);
          sock->SetTimeout(5);
          CZNC::Get().GetManager().SwapSockByAddr(sock, &WebSock);
          sock->SetSockName("WebClient::Events");
          m_sockets.push_back(sock);
          return true;
	}

	virtual bool OnWebRequest(CWebSock& WebSock, const CString& sPageName, CTemplate& Tmpl) {
		if (sPageName == "index") {
                  //TODO: send existing user info/channel etc
                  CUser* user = GetUser();
                  const vector<CIRCNetwork*>& networks = user->GetNetworks();
                  for (unsigned int i = 0; i < networks.size(); i++) {
                    CTemplate& n = Tmpl.AddRow("NetworkLoop");
                    n["Network"] = networks[i]->GetName();
                    n["Nick"] = networks[i]->GetCurNick();

                    const vector<CChan*>& channels = networks[i]->GetChans();
                    for (unsigned int j = 0; j < channels.size(); j++) {
                      CTemplate& c = n.AddRow("ChannelLoop");
                      c["Channel"] = channels[j]->GetName();
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
