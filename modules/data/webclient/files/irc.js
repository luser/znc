function IRCNetwork(name, nick, channels) {
  this.name = name;
  this.nick = nick;
  this.channels = {};
  for (var i = 0; i < channels.length; i++)
    this.channels[channels[i].name] = channels[i];
  this.privmsgs = {};
}

IRCNetwork.prototype = {
  toString: function() {
    return "[IRCNetwork " + this.name + "]";
  },

  OnPrivmsg: function(user, msg, action, self) {
    if (!(user in this.privmsgs))
      this.privmsgs[user] = [];
    this.privmsgs[user].push({msg: msg,
                              action: action ? true : false,
                              self: self ? true : false});
  }
};

function Channel(name) {
  this.name = name;
  this.scrollback = [];
}

Channel.prototype = {
  toString: function() {
    return "[Channel " + this.name + "]";
  },

  OnMessage: function(user, msg, action, self) {
    this.scrollback.push({user: user,
                          msg: msg,
                          type: action ? "action" : "message",
                          self: self ? true : false});
  }
};
