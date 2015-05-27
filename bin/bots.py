#!/usr/bin/env python
# coding: utf-8
# Copyright (C) 2007-2009 Centro de Computacao Cientifica e Software Livre
# Departamento de Informatica - Universidade Federal do Parana - C3SL/UFPR
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
# USA.

# bots.py
# Connects and manages interaction of chess engine(s) with a chess server.
# Translate between CECP (Chess Engine Communication Protocol) and a chess
# protocol based on XMPP.
# For more information, please visit http://xadrezlivre.c3sl.ufpr.br/

from threading import *
from xml.dom.minidom import parseString, parse
from xml.parsers.expat import ExpatError
import socket, os, sys, re, select, getopt, random, exceptions, Queue, time

########## <PROTOCOL> ###########
# Jabber Resource
resource = "ChessD"

# BIND tags
BODY = "<body rid='%s' sid='%s' xmlns='http://jabber.org/protocol/httpbind'/>"
BODY_HEAD = "<body rid='%s' sid='%s' xmlns='http://jabber.org/protocol/httpbind'>"
BODY_TAIL = "</body>"
BODY_TERMINATE = "<body rid='%s' sid='%s' type='terminate' xmlns='http://jabber.org/protocol/httpbind'/>"

# Connection steps
# Step 1, get a SID from the Bind Server
ASK_SID = "<body hold='1' rid='%d' to='%s' ver='1.6' wait='10' xml:lang='en' xmlns='http://jabber.org/protocol/httpbind'/>"
# Step 2, get authentication parameters from Jabber Server
CONNECT_1 = "<iq type='get' id='auth_1' to='%s'><query xmlns='jabber:iq:auth'><username>%s</username></query></iq>"
# Step 3, send required authentication info to the Jabber Server
CONNECT_2 = "<iq type='set' id='auth_2' to='%s'><query xmlns='jabber:iq:auth'><username>%s</username><password>%s</password><resource>%s</resource></query></iq>"
# Last step, sends Presences to everyone, to the general chat room and to
# the matches component
GLOBAL_PRESENCE = "<presence from='%s'/><presence to='general@conference.%s/%s'/><presence to='chessd.%s'><config multigame='true'/></presence>"

UPDATE_PROFILE = "<iq type='set'><vCard xmlns='vcard-temp' prodid='-//HandGen//NONSGML vGen v1.0//EN' version='2.0'><FN>%s</FN><DESC></DESC><PHOTO><TYPE></TYPE><BINVAL></BINVAL></PHOTO></vCard></iq>"

# Disconnection
DISCONNECT = "<presence xmlns='jabber:client' type='unavailable'><status>Logged out</status></presence>"

# Match offer
OFFER_MATCH = "<iq type='set' to='chessd.%s' id='match'><query xmlns='http://c3sl.ufpr.br/chessd#match#offer'><match category='blitz'><player inc='0' color='white' time='180' jid='%s'/><player inc='0' color='black' time='180' jid='%s'/></match></query></iq>"

# Accept match offer
ACCEPT_MATCH = "<iq type='set' to='chessd.%s' id='match'><query xmlns='http://c3sl.ufpr.br/chessd#match#accept'><match id='%d'/></query></iq>"

# Decline match offer
DECLINE_MATCH = "<iq type='set' to='chessd.%s' id='match'><query xmlns='http://c3sl.ufpr.br/chessd#match#decline'><match id='%d'/></query></iq>"

# Join the game room
JOIN_GAME= "<presence to='%s@chessd.%s/%s'/>"

# Leave the game room
LEAVE_GAME = "<presence to='%s@chessd.%s/%s' type='unavailable'/>"

# Make a move in the game
MOVE = "<iq type='set' to='%s@chessd.%s' id='match'><query xmlns='http://c3sl.ufpr.br/chessd#game#move'><move long='%s'/></query></iq>"

# Reply to draw/cancel/adjourn messages
ACCEPT_ENDGAME = "<iq type='set' from='%s' to='%s@chessd.%s' id='%s'><query xmlns='http://c3sl.ufpr.br/chessd#game#%s'/></iq>"

# Private chat message
MESSAGE = "<message from='%s' to='%s' type='chat'><body>%s</body></message>"

# Subscribed
SUBSCRIBED = "<presence from='%s' to='%s' type='subscribed'><status/></presence>"
########## </PROTOCOL> ###########

# HTTP packet (header+data) to be POSTed on each HTTP communication
HTTP_POST = "POST /jabber HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\n\r\n%s"

# Initial Chess Board in FEN (Forsyth-Edwards Notation)
DEFAULT_BOARD = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"

# Main function. Loads everything, launchs threads and waits "forever"
def main():
    # Check in-line parameters
    config = check_args()
    if not "config_file" in config.keys():
        config["config_file"] = "config.xml"
    # Loads configuration file
    config = load_config(config)
    # Properly sets configuration options
    set_config(config)
    # Start messages logging
    start_log()
    # Lock variable, used only for the wait method
    cond = Condition()
    try:
        # Start threads for each configured Bot users to the chess server
        start_bots()
        cond.acquire()
        while True:
            cond.wait(1)
        cond.release()
    except KeyboardInterrupt:
        stop_bots()

# Check and process inline arguments (can set server, port, log and conf files)
def check_args():
    config = {}
    short = "s:p:l:c:o:h"
    long = ["server=", "port=", "log=", "config=", "help"]
    try:
        opts, args = getopt.getopt(sys.argv[1:], short, long)
    except getopt.GetoptError:
        usage()
        sys.exit(1)
    for opt, val in opts:
        if opt in ("-s", "--server"):
            config["server"] = val
        elif opt in ("-p", "--port"):
            config["port"] = val
        elif opt in ("-l", "--log"):
            config["log_file"] = val
        elif opt in ("-c", "--config"):
            config["config_file"] = val
        elif opt in ("-h", "--help"):
            usage()
            sys.exit(1)
    return config


# Loads the configuration file (config.xml by default)
def load_config(config):
    # try to open config file and parses the XML string
    try:
        tree = parse(config["config_file"])
    except IOError:
        print "[ERROR] Could not open configuration file '%s'!" % (config["config_file"],)
        sys.exit(1)
    try:
        node = tree.getElementsByTagName("bots")[0]
        # Get server address/IP
        if node.hasAttribute("server"):
            if not "server" in config.keys():
                config["server"] = node.getAttribute("server")
        # Get server port
        if node.hasAttribute("port"):
            if not "port" in config.keys():
                config["port"] = int(node.getAttribute("port"))
        # Get log file path
        if node.hasAttribute("log"):
            if not "log_file" in config.keys():
                config["log_file"] = node.getAttribute("log")

        # Get info for each bot (user, password, path to chess engine)
        config["bots"] = []
        for node in node.getElementsByTagName("bot"):
            user = node.getAttribute("username")
            passw = node.getAttribute("password")
            engine_path = node.getAttribute("enginepath")
            bot_opp = node.getAttribute("opponent")
            bot = (user, passw, engine_path, bot_opp)
            config["bots"].append(bot)
    except:
        print "[ERROR] Could not parse config file '%s'!" % (config["config_file"],)
        raise
        sys.exit(1)

    return config

# Setup the configured options
def set_config(config):
    global server, port, log_file, bots
    # Set server address/IP
    if "server" in config.keys():
        server = config["server"].encode('utf8')
    else:
        print "[ERROR] Missing server configuration!"
        sys.exit(1)
    # Set server port
    if "port" in config.keys():
        port = int(config["port"])
    else:
        print "[ERROR] Missing port configuration!"
        sys.exit(1)
    # Set log file path (optional)
    if "log_file" in config.keys():
        log_file = config["log_file"]
    else:
        log_file = None
    # Start bots' structures
    bots = []
    for bot in config["bots"]:
        user, passw, engine_path, opponent = bot
        b = Bot(user, passw, engine_path, opponent)
        bots.append(b)

# Try to open log file in append mode
def start_log():
    global log_fd, log_file
    log_fd = None
    if log_file:
        try:
            log_fd = open(log_file, 'a')
        except IOError:
            log("[ERROR] Could not open log file '%s'!" % (log_file,))
    log("Bots started!")

# Logs a message to a file (if opened) and also to the screen
def log(msg):
    global log_fd
    msg = (time.strftime("%c", time.localtime()) + " " + msg).encode('utf8')
    if log_fd:
        log_fd.write(msg+"\n")
        log_fd.flush()
    print msg

# Start all configured bots
def start_bots():
    global bots
    if len(bots):
        i = 0
        for bot in bots:
            t = Timer(i/4, bot.start)
            t.start()
            i = i + 1
    else:
        log("[ERROR] There are no bots to start! Check configuration file.")
        sys.exit(1)

# Stop all running bots
def stop_bots():
    for bot in bots:
        if bot.isAlive():
            bot.stop()
            bot.join()

# Prints the usage instructions
def usage():
    print "Usage: bots.py [OPTION]\
        \nConnects and manages interaction between chess engine(s) and a chess server.\
        \nVisit http://xadrezlivre.c3sl.ufpr.br/\
        \n\
        \n  -s, --server <address>\tSelects chess server's Address/IP\
        \n  -p, --port <server_port>\tSelect chess server's port\
        \n  -l, --log <log_file>\t\tSet log file path\
        \n  -c, --config <config_file>\tSet configuration file path\
        \n  -h, --help\t\t\tShow this help message"

# A Chess Bot.
# Connects to the server, play chess games (more than 1 at a time,
# reply private chat msgs. Runs in a separate Thread.
class Bot(Thread):
    def __init__(self, user, passw, engine_path, opponent):
        Thread.__init__(self)
        self.user = user
        self.passw = passw
        self.jid = user+"@"+server+"/"+resource
        self.rid = 0
        self.sid = None
        self.sid_asked = False
        self.timeout = 2
        self.authenticating = False
        self.online = False
        self.sockets = []
        self.idle_sockets = []
        self.msg_queue = []
        self.engine_path = engine_path
        self.exit = False
        self.cond = Condition()
        self.matches = {}
        self.games = {}
        self.last_offer = None
        self.opp = opponent
        self.opp_online = False

    # Chess bot thread's main function
    def run(self):
        last_time_received = time.time() + 120
        while True:
            # Run this thread until self.exit is True
            self.cond.acquire()
            if self.exit:
                # Disconnect from Jabber Server
                self.disconnect(True)
                self.cond.release()
                return
            self.cond.release()

            # If we don't have a sid, asks for it
            if self.sid == None:
                if not self.sid_asked:
                    self.ask_sid()
            # If not connected, connect
            elif not self.online and not self.authenticating:
                self.connect()
            # If there are no messages to send and all the open sockets are idle
            if len(self.msg_queue) == 0 and len(self.sockets) == len(self.idle_sockets):
                # Insert an empty message into the send queue
                self.send("")

            # Attempt to actually send the first msg in the send queue
            self.http_post()

            p = select.poll()
            # List of sockets to wait for reading
            for sock in self.sockets:
                p.register(sock, select.POLLIN)
            bot_sock = []
            for game in self.games.itervalues():
                bot_sock.append(game["engine"].r_bot)
                p.register(game["engine"].r_bot, select.POLLIN)

            # Wait for incoming messages, if any
            r = p.poll(10000)

            # If there is a message to receive, receive it
            for sock, event in r:
                for s in self.sockets:
                    if s.fileno() == sock:
                        self.recv(s)
                        last_time_received = time.time()

            # If there are any moves from the engines, send them
            for room, game in self.games.iteritems():
                if (game["engine"].r_bot.fileno(), select.POLLIN) in r and game["engine"].r_bot in bot_sock:
                    move = game["engine"].r_bot.recv(16)
                    self.send_move(room, move)

            # If there is an opponent to challenge, offer him/her a match
            self.challenge()

            # If there wasn't any incoming messages in the past 60s, disconnect
            if self.online or self.sid != None:
                if time.time() - last_time_received >= 60:
                    log("[%s] Closing connection due to inactivity..." % (self.user,))
                    self.disconnect(False)

    # Stops the bot thread
    def stop(self):
        self.cond.acquire()
        self.exit = True
        self.cond.release()

    # If we don't have a SID from the Bind server, asks for it
    def ask_sid(self):
        self.rid = random.randint(0,9999999)
        self.sid = None
        log("[%s] Asking a SID from the Bosh server" % (self.user,))
        self.send(ASK_SID % (self.rid, server), body=False)
        self.sid_asked = True
        t = Timer(self.timeout, self.retry)
        t.start()

    # If the connection's first step was succesful. In negative case, define
    # the timeout for restarting the connection process
    def retry(self):
        if self.sid != None:
            self.timeout = 10
        else:
            if self.timeout < 60:
                self.timeout = self.timeout + 10 + random.randint(0,10)
        self.sid_asked = False

    # Start the connection's process with the Jabber server
    def connect(self):
        self.send(CONNECT_1 % (server, self.user))
        self.authenticating = True

    # Finish the connection with the Jabber server
    # If the parameter clean is True, sends messages ending games
    # and terminating connection with Bosh server before closing the sockets
    def disconnect(self, clean):
        if self.online or self.sid != None:
            # Close any engine threads that may be open
            for room, game in self.games.iteritems():
                # If we are still connected, leave any open game room
                if clean:
                    self.send(LEAVE_GAME % (room, server, self.user), now=True)
                game["engine"].stop()
            self.matches = {}
            self.games = {}
            # If still connected, sends disconnection message
            if clean:
                self.send(BODY_TERMINATE % (self.rid, self.sid), body=False, now=True)
            # Set status as offline
            self.sid = None
            self.online = False
            self.authenticating = False
            self.opp_online = False
            # Clear the msg queue
            while len(self.msg_queue):
                self.msg_queue.remove()
            # Close any open socket
            for sock in self.sockets:
                sock.close()
                self.sockets.remove(sock)
            self.idle_sockets = []
            log("[%s] Disconnected from server '%s'" % (self.user, server))

    # Schedule a XMPP message to be sent to the Jabber server
    # If the argument 'now' is True, send immediately
    def send(self, msg, body=True, now=False):
        if body:
            if self.sid == None:
                return
            if len(msg) > 0:
                msg = (BODY_HEAD + msg + BODY_TAIL) % (self.rid, self.sid)
            else:
                msg = BODY % (self.rid, self.sid)
        self.msg_queue.append(msg)
        self.rid = self.rid + 1
        if now:
            self.http_post()

    # Sends a move command that came from the Engine
    def send_move(self, room, move):
        if self.online and room in self.games.keys():
            self.send(MOVE % (room, server, move), now=True)
            log("[%s] [%s] Sending move '%s'" % (self.user, room, move))

    # Post a XMPP message via HTTP
    def http_post(self):
        if len(self.msg_queue):
            msg = self.msg_queue.pop(0)
        else:
            return
        msg = msg.encode('utf8')
        for sock in self.sockets:
            if sock in self.idle_sockets:
                self.idle_sockets.remove(sock)
                try:
                    sock.sendall(HTTP_POST % (server, len(msg), msg))
                    return
                except:
                    sock.close()
                    self.sockets.remove(sock)
                    continue
        if len(self.sockets) < 2:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                sock.connect((server, port))
            except socket.error, e:
                log("[%s] [ERROR] Server '%s' is not responding [Exception: %s]" % (self.user, server, e))
                self.disconnect(False)
                return
            try:
                sock.sendall(HTTP_POST % (server, len(msg), msg))
            except Exception, e:
                log("[%s] [ERROR] Server '%s' is not responding [Exception: %s]" % (self.user, server, e))
                self.disconnect(False)
                return
            self.sockets.append(sock)
        else:
            self.msg_queue.insert(0, msg.decode('utf8'))

    # Receive a XMPP message from the server, getting rid of HTTP header
    def recv(self, sock, buffer_len=1024):
        try:
            msg = sock.recv(buffer_len)
        except:
            sock.close()
            self.sockets.remove(sock)
            if sock in self.idle_sockets:
                self.idle_sockets.remove(sock)
            return
        if len(msg) == 0:
            sock.close()
            self.sockets.remove(sock)
            if sock in self.idle_sockets:
                self.idle_sockets.remove(sock)
            return

        while len(msg) > 0:
            # Receive the HTTP header, at least
            while len(msg.split("\r\n\r\n")) < 2:
                chunk = sock.recv(buffer_len)
                if len(chunk) == 0:
                    sock.close()
                    self.sockets.remove(sock)
                    if sock in self.idle_sockets:
                        self.idle_sockets.remove(sock)
                    return
                msg = msg + chunk

            # Separates the HTTP header from the entire message
            header = msg.split("\r\n\r\n")[0]
            msg = msg[(len(header) + 4):]

            # Get the length of the packet (data section)
            m = re.compile(r"Content-Length:\s+(\d+)").search(header)
            if not m:
                sock.close()
                self.sockets.remove(sock)
                if sock in self.idle_sockets:
                    self.idle_sockets.remove(sock)
                return
            content_len = int(m.group(1))

            # Receive the rest of the packet, if it is not complete yet
            while content_len > len(msg):
                chunk = sock.recv(content_len - len(msg))
                if len(chunk) == 0:
                    sock.close()
                    self.sockets.remove(sock)
                    if sock in self.idle_sockets:
                        self.idle_sockets.remove(sock)
                    return
                msg = msg + chunk

            # Parses the XMPP (Jabber) message
            if content_len < len(msg):
                self.parser(msg[:content_len])
                msg = msg[content_len:]
            else:
                self.parser(msg)
                msg = ""
        if not sock in self.idle_sockets:
            self.idle_sockets.append(sock)


    # If an opponent is defined, offer him a match, if not already playing
    # Intended to Debug purposes
    def challenge(self, time=180, inc=0, category="blitz"):
        if len(self.opp) > 0 and len(self.matches) == 0 and len(self.games) == 0 and not self.last_offer and self.online and self.opp_online:
            if random.randint(0,1):
                p1_jid = self.jid
                p2_jid = self.opp+"@"+server+"/"+resource
            else:
                p1_jid = self.opp+"@"+server+"/"+resource
                p2_jid = self.jid
            self.send(OFFER_MATCH % (server, p1_jid, p2_jid))
            self.last_offer = {"category": category, "p1_jid": p1_jid, "p1_time": time, "p1_inc": inc, "p1_color": "white", "p2_jid": p2_jid, "p2_time": time, "p2_inc": inc, "p2_color": "black"}
            log("[%s] Offering match %s vs %s" % (self.user, p1_jid, p2_jid))


    # Parse a XMPP string, taking the needed actions according to it
    def parser(self, xmpp):
        try:
            # Creates the parsing structure
            node = parseString(xmpp)
            # <error>...</error>
            if node.firstChild.tagName == "error":
                if node.firstChild.firstChild.data == "invalid sid":
                    log("[%s] Disconnected from Bosh server" % (self.user,))
                    self.disconnect(False)
                    return

            # <body ...>
            node = node.getElementsByTagName("body")[0]
            # If SID is not defined, define the new one
            if self.sid_asked and self.sid == None:
                if not node.getAttribute("sid") == None:
                    if len(node.getAttribute("sid")) > 0:
                        self.sid = node.getAttribute("sid")
                        log("[%s] Acquired SID \"%s\"" % (self.user, self.sid))
            # If the Bosh server terminates the connection, close the
            # connection with the Jabber server
            body_type = node.getAttribute("type")
            body_condition = node.getAttribute("condition")
            if body_type == "terminate":
                log("[%s] Disconnected from Bosh server (connection terminated, condition '%s')" % (self.user, body_condition))
                log(xmpp)
                self.disconnect(False)
                return

            for node in node.childNodes:
                # <message ...>
                if node.tagName == "message":
                    # Private chat message
                    if node.getAttribute("type") == "chat":
                        chat_from = node.getAttribute("from")
                        self.send(MESSAGE % (self.jid, chat_from, "(auto-resposta) Oi, eu sou um computador que joga Xadrez! Não sei conversar!".decode('utf8')))
                        node = node.getElementsByTagName("body")[0]
                        msg = node.firstChild.data
                        log("[%s] Message from '%s': '%s'" % (self.user, chat_from, msg))
                    # Group chat message, tipically a chat room.
                    elif node.getAttribute("type") == "groupchat":
                        # Ignore chat room messages
                        chat_from = node.getAttribute("from")
                        if chat_from.count("/") == 1:
                            room, chat_jid = chat_from.split("/")
                        node = node.getElementsByTagName("body")[0]
                        msg = node.firstChild.data
                # <presence ...>
                if node.tagName == "presence":
                    presence_from = node.getAttribute("from")
                    presence_to = node.getAttribute("to")
                    presence_type = node.getAttribute("type")
                    # Someone asking for subscription
                    if presence_type == "subscribe":
                        self.send(SUBSCRIBED % (presence_to, presence_from))
                        log("[%s] Authorized contact '%s'" % (self.user, presence_from))
                    if presence_from == "general@conference."+server+"/"+self.opp:
                        if presence_type == "unavailable":
                            log("[%s] Opponent '%s' is offline!" % (self.user, self.opp))
                            self.opp_online = False
                        else:
                            log("[%s] Opponent '%s' is online!" % (self.user, self.opp))
                            self.opp_online = True
                # <iq ...>
                if node.tagName == "iq":
                    # Stores iq's attributes
                    iq_from = node.getAttribute("from")
                    iq_type = node.getAttribute("type")
                    iq_id = node.getAttribute("id")
                    iq_to = node.getAttribute("to")

                    # Reply of the first Jabber connection step? Sends 2nd step
                    if iq_id == "auth_1" and iq_type == "result" and iq_from == server:
                        self.send(CONNECT_2 % (server, self.user, self.passw, resource))
                    # Authenticated by the Jabber Server? Sends global presence
                    elif iq_id == "auth_2" and iq_type == "result" and iq_from == server:
                        self.authenticating = False
                        self.online = True
                        self.send(GLOBAL_PRESENCE % (self.jid, server, self.user, server))
                        self.send(UPDATE_PROFILE % (self.user,))
                        log("[%s] Connected to server '%s'" % (self.user, server))

                    # In case of no child node, skip this iq
                    if not node.hasChildNodes():
                        continue
                    # Stores xmlns attribute from the 'query' tag
                    query = node.getElementsByTagName("query")[0]
                    xmlns = query.getAttribute("xmlns")

                    # If it has been any kind of error, move on with the parsing
                    if iq_type == "error":
                        if xmlns in ("http://c3sl.ufpr.br/chessd#game#move","http://c3sl.ufpr.br/chessd#game#cancel"):
                            continue
                        room = iq_from.split("@")[0]
                        error = node.getElementsByTagName("error")[0]
                        error_code = error.getAttribute("code")
                        error_type = node.getAttribute("type")
                        log("[%s] [%s] [ERROR] [query xmlns='%s'] [error code='%s' type='%s']" % (self.user, room, xmlns, error_code, error_type))
                        log("[%s] Disconnecting due to error" % (self.user,))
                        log(xmpp)
                        self.disconnect(True)
                        return

                    # Match offer
                    if xmlns == "http://c3sl.ufpr.br/chessd#match#offer" and iq_type == "set":
                        node = node.getElementsByTagName("match")[0]
                        # Match category (standard, blitz, etc)
                        match_category = node.getAttribute("category")
                        match_id = int(node.getAttribute("id"))
                        # Player 1 data
                        p1 = node.getElementsByTagName("player")[0]
                        p1_jid = p1.getAttribute("jid")
                        p1_time = p1.getAttribute("time")
                        p1_inc = p1.getAttribute("inc")
                        p1_color = p1.getAttribute("color")
                        # Player 2 data
                        p2 = node.getElementsByTagName("player")[1]
                        p2_jid = p2.getAttribute("jid")
                        p2_time = p2.getAttribute("time")
                        p2_inc = p2.getAttribute("inc")
                        p2_color = p2.getAttribute("color")
                        # Save match info
                        self.matches[match_id] = {"category": match_category,"p1_jid": p1_jid, "p1_time": p1_time, "p1_inc": p1_inc, "p1_color": p1_color, "p2_jid": p2_jid, "p2_time": p2_time, "p2_inc": p2_inc, "p2_color": p2_color}
                        self.send(ACCEPT_MATCH % (server, match_id))
                        log("[%s] Accepting match '%s'" % (self.user, match_id))
                    # Result from match offer
                    elif xmlns == "http://c3sl.ufpr.br/chessd#match#offer" and iq_type == "result":
                        node = node.getElementsByTagName("match")[0]
                        match_id = int(node.getAttribute("id"))
                        self.matches[match_id] = self.last_offer
                        self.last_offer = None
                    # A match has been accepted
                    elif xmlns == "http://c3sl.ufpr.br/chessd#match#accept":
                        node = node.getElementsByTagName("match")[0]
                        match_id = int(node.getAttribute("id"))
                        match_room = node.getAttribute("room")
                        room = match_room.split("@")[0]
                        # Save game's room name 
                        self.games[room] = self.matches[match_id]
                        # Remove match offer from match list
                        del self.matches[match_id]
                        # Retrieve opponent's settings
                        if self.games[room]["p1_jid"] == self.jid:
                            color = self.games[room]["p1_color"]
                            opp_jid = self.games[room]["p2_jid"]
                            opp_color = self.games[room]["p2_color"]
                        else:
                            color = self.games[room]["p2_color"]
                            opp_jid = self.games[room]["p1_jid"]
                            opp_color = self.games[room]["p1_color"]
                        if color == "white":
                            self.games[room]["is_white"] = True
                        elif color == "black":
                            self.games[room]["is_white"] = False
                        # Set up Chess Engine (one for each game)
                        engine = Engine(self.engine_path)
                        engine.start()
                        self.games[room]["engine"] = engine
                        self.games[room]["wait_1st_board"] = True
                        self.send(JOIN_GAME % (room, server, self.user))
                        log("[%s] [%s] Starting game: %s (%s) vs %s (%s)" % (self.user, room, self.user, color, opp_jid.split("@")[0], opp_color))
                    elif xmlns == "http://c3sl.ufpr.br/chessd#match#decline":
                        node = node.getElementsByTagName("match")[0]
                        match_id = int(node.getAttribute("id"))
                        # Remove match from matches queue
                        del self.matches[match_id]
                        log("[%s] Match '%s' declined" % (self.user, match_id))

                    # Messages about game features (move, board state, etc)
                    # First board received
                    elif xmlns == "http://c3sl.ufpr.br/chessd#game#state":
                        board = node.getElementsByTagName("board")[0]
                        fm = board.getAttribute("fullmoves")
                        hm = board.getAttribute("halfmoves")
                        enp = board.getAttribute("enpassant")
                        castle = board.getAttribute("castle")
                        pp = board.getAttribute("state")
                        turn = board.getAttribute("turn")
                        color = turn[0]
                        room = iq_from.split("@")[0]
                        # If the players' colors are undefined, retrieve them
                        if "is_white" not in self.games[room]:
                            # Player 1 color
                            p1 = node.getElementsByTagName("player")[0]
                            p1_color = p1.getAttribute("color")
                            p1_jid = p1.getAttribute("jid")
                            # Player 2 color
                            p2 = node.getElementsByTagName("player")[1]
                            p2_color = p2.getAttribute("color")
                            p2_jid = p2.getAttribute("jid")
                            # Set colors in the right places
                            if self.games[room]["p1_jid"] == p1_jid:
                                self.games[room]["p1_color"] = p1_color
                                self.games[room]["p2_color"] = p2_color
                            else:
                                self.games[room]["p1_color"] = p2_color
                                self.games[room]["p2_color"] = p1_color
                            # Finally, set the 'is_white' attribute
                            if self.games[room]["p1_jid"] == self.jid:
                                color = self.games[room]["p1_color"]
                            else:
                                color = self.games[room]["p2_color"]
                            if color == "white":
                                self.games[room]["is_white"] = True
                            else:
                                self.games[room]["is_white"] = False
                        # Tell engine the initial board
                        if self.games[room]["wait_1st_board"]:
                            self.games[room]["wait_1st_board"] = False
                            # if the match is timed
                            if self.games[room]["category"] != "untimed":
                                # Set the times accordingly to the match times
                                if self.jid == self.games[room]["p1_jid"]:
                                    self.games[room]["engine"].set_time(self.games[room]["p1_time"], self.games[room]["p1_inc"])
                                else:
                                    self.games[room]["engine"].set_time(self.games[room]["p2_time"], self.games[room]["p2_inc"])
                            # board different from default start position?
                            if not pp == DEFAULT_BOARD:
                                self.games[room]["engine"].setboard(pp, color, castle, enp, hm, fm)
                            # Tell the engine to begin playing accordingly
                            # with the bot color
                            self.games[room]["engine"].play(turn, self.games[room]["is_white"])
                            log("[%s] [%s] Received first board, game started!" % (self.user, room))

                    # Movement from the oponent
                    elif xmlns == "http://c3sl.ufpr.br/chessd#game#move":
                        if iq_type == "set":
                            move = node.getElementsByTagName("move")[0]
                            long = move.getAttribute("long")
                            board = node.getElementsByTagName("board")[0]
                            fm = board.getAttribute("fullmoves")
                            hm = board.getAttribute("halfmoves")
                            enp = board.getAttribute("enpassant")
                            castle = board.getAttribute("castle")
                            pp = board.getAttribute("state")
                            turn = board.getAttribute("turn")
                            color = turn[0]
                            room = iq_from.split("@")[0]
                            if (turn == "white" and self.games[room]["is_white"]) or (turn == "black" and not self.games[room]["is_white"]):
                                self.games[room]["engine"].usermove(long)
                                log("[%s] [%s] Received move '%s' fm: %s" % (self.user, room, long, fm))

                    # Opponent has resigned
                    elif xmlns == "http://c3sl.ufpr.br/chessd#game#resign":
                        log("[%s] [%s] Opponent has resigned!" % (self.user, room))
                    # Opponent asking for end of game by draw
                    elif xmlns == "http://c3sl.ufpr.br/chessd#game#draw":
                        room = iq_from.split("@")[0]
                        self.games[room]["engine"].send("draw\n");
                        # Schedule to verify the answer from the Engine about
                        # the agreement or disagreement with the draw request
                        t = Timer(2.0, self.verify_draw, [room])
                        t.start()

                    # Opponent asking for end of game by cancel/adjourn
                    elif xmlns in ("http://c3sl.ufpr.br/chessd#game#cancel", "http://c3sl.ufpr.br/chessd#game#adjourn"):
                        action = xmlns.split("#")[2]
                        room = iq_from.split("@")[0]
                        self.send(ACCEPT_ENDGAME % (self.jid, room, server, action, action))
                        log("[%s] [%s] Accepted '%s' request" % (self.user, room, action))
                    # Game ended
                    elif xmlns == "http://c3sl.ufpr.br/chessd#game#end":
                        room = iq_from.split("@")[0]
                        end = node.getElementsByTagName("end")[0]
                        end_type = end.getAttribute("type")
                        end_result = end.getAttribute("result")
                        # Game ended normally
                        if end_type == "normal":
                            p1 = node.getElementsByTagName("player")[0]
                            p1_color = p1.getAttribute("role")
                            p1_result = p1.getAttribute("result")
                            p1_jid = p1.getAttribute("jid")
                            p2 = node.getElementsByTagName("player")[1]
                            p2_result  = p2.getAttribute("result")
                            p2_jid = p2.getAttribute("jid")
                            results = {"won" : "1-0", "lost" : "0-1", "draw" : "1/2-1/2"}
                            result = results[p1_result]
                            if p1_color == "white":
                                white = p1_jid.split("@")[0]
                                black = p2_jid.split("@")[0]
                            else:
                                white = p2_jid.split("@")[0]
                                black = p1_jid.split("@")[0]
                            # Tell engine the result
                            self.games[room]["engine"].send("result %s {%s}\n" % (result, end_result))
                            log("[%s] [%s] Game ended: %s %s %s, reason: %s " % (self.user, room, white, result, black, end_result))
                        # Game Adjourned
                        elif end_type == "adjourned":
                            log("[%s] [%s] Game Adjourned!" % (self.user, room))
                        # Game Canceled
                        elif end_type == "canceled":
                            log("[%s] [%s] Game Canceled!" % (self.user, room))
                        # Stops the chess engine
                        self.games[room]["engine"].stop()
                        # Removes the game from the games list
                        del self.games[room]
                        # Leave the game room
                        self.send(LEAVE_GAME % (room, server, self.user))

                    # Jabber server Authentication
                    elif xmlns == "jabber:iq:auth":
                        username = node.getElementsByTagName("username")[0]
                        if self.user != username.firstChild.data:
                            log("[%s] [ERROR] Authentication error!" % (self.user,))
                    # Ignores any roster message
                    elif xmlns == "jabber:iq:roster":
                        pass

                    # Ignores any Discovery info query
                    elif xmlns == "http://jabber.org/protocol/disco#info":
                        pass
                    else:
                        log("[%s] [ERROR] Unknown query's xmlns '%s', iq's type '%s'" % (self.user, xmlns, iq_type))
        except ExpatError:
            log("[%s] [ERROR] The recieved XMPP is not well-formed. This could mean that the server is down or not reachable." % (self.user,))
            self.disconnect(False)
            return
        except Exception, e:
            log("[%s] [ERROR] Unable to process correctly the following XMPP [Exception: %s]" % (self.user, e))
            log(xmpp)
            self.disconnect(False)
            return

    def verify_draw(self, room):
        action = "draw"
        if room in self.games.keys() and self.games[room]["engine"].accepted_draw:
            self.send(ACCEPT_ENDGAME % (self.jid, room, server, action, action))
            log("[%s] [%s] Accepted '%s' request" % (self.user, room, action))
        else:
            log("[%s] [%s] Rejected '%s' request" % (self.user, room, action))

# A Chess Engine.
# Forks an execution of a chess engine process, handling messages of
# Chess Engine Communication Protocol with it. Runs in a separate Thread.
# There is one Engine instance for each game that a Bot plays
class Engine(Thread):
    def __init__(self, path):
        Thread.__init__(self)
        self.running = False
        self.r_sock = None
        self.w_sock = None
        self.ping_id = 0
        self.pid = 0
        self.path = path
        self.cond = Condition()
        # Setup socket pair, so the Engine can send moves
        self.r_bot, self.w_bot = socket.socketpair(socket.AF_UNIX, socket.SOCK_STREAM)
        self.feature_usermove = False
        self.feature_colors = True
        self.feature_ping = False
        self.accepted_draw = False
        self.accepted_done = False
        self.msg_queue = Queue.Queue()

    # Engine Thread's main function
    def run(self):
        # Run the Chess Engine Process
        self.execute()
        while True:
            # If it is time to stop, just return
            self.cond.acquire()
            if not self.running:
                self.cond.release()
                return
            self.cond.release()

            p = select.poll()
            p.register(self.r_sock, select.POLLIN)
            try:
                # Wait for messages from the Engine process
                r = p.poll(1000)
            except select.error, err:
                if err[1] == "Bad file descriptor":
                    continue

            # If there are anything to receive, receive it
            if (self.r_sock, select.POLLIN) in r:
                self.recv()

            self.cond.acquire()
            while not self.msg_queue.empty():
                if self.running and self.accepted_done:
                    # Try to send a msg from the queue
                    msg = self.msg_queue.get(False)
                    try:
                        os.write(self.w_sock, msg)
                    except Exception, e:
                        log("[pid %d] [ERROR] Unable to write in socket: '%s'" % (self.pid, e))
                        break
                else:
                    break
            self.cond.release()

    # Run the Chess Engine Process, saving the File Desriptors for I/O
    def execute(self):
        try:
            r = os.pipe()
            w = os.pipe()
            self.pid = os.fork()
            if self.pid == 0:
                os.dup2(r[0], sys.stdin.fileno())
                os.dup2(w[1], sys.stdout.fileno())
                os.close(r[0])
                os.close(r[1])
                os.close(w[0])
                os.close(w[1])
                args = self.path.split()
                os.execvp(args[0], args)
            else:
                os.close(r[0])
                os.close(w[1])
                self.w_sock = r[1]
                self.r_sock = w[0]
        except exceptions.OSError, e:
            log("[ERROR] Could not run Chess Engine at '%s': '%s'" % (self.path, e))
            sys.exit(1)
        # Initial CECP commands sent to the Chess Engine
        try:
            os.write(self.w_sock, "xboard\nprotover 2\n")
        except Exception, e:
            log("[pid %d] [ERROR] Unable to write in socket: '%s'" % (self.pid, e))
            return
        self.running = True
        log("[pid %d] Chess Engine '%s' started" % (self.pid, self.path))

    # Stops a Chess Engine proccess
    def stop(self):
        self.cond.acquire()
        if self.running:
            # Close FD's
            os.close(self.w_sock)
            os.close(self.r_sock)
            # Kill the process
            os.kill(self.pid, 9)
            os.waitpid(self.pid, 0)
            self.running = False
            log("[pid %d] Chess Engine '%s' stopped" % (self.pid, self.path))
        self.cond.release()

    # Sends a ping message to the Chess Engine
    def ping(self):
        if self.feature_ping:
            if not self.ping_id:
                self.ping_id = 1
            else:
                self.ping_id += 1
            self.send("ping %d\n" % (self.ping_id,))

    # Sends a movement from the opponent (real user)
    def usermove(self, long):
        if self.feature_usermove:
            self.send("usermove %s\n" % (long,))
        else:
            self.send("%s\n" % (long,))

    # Setup a board position
    def setboard(self, pp, turn, castle, enp, hm, fm):
        self.send("setboard %s %s %s %s %s %s" % (pp, turn, castle, enp, hm, fm))

    # Set the engine to begin playing, or to wait a user move, according to
    # the player's color on move
    def play(self, turn, is_white):
        self.send("force\nnew\nrandom\n")
        if is_white:
            if self.feature_colors:
                if turn == "white":
                    self.send("black\nwhite\ngo\n")
                elif turn == "black":
                    self.send("black\n")
            else:
                if turn == "white":
                    self.send("go\n")
                elif turn == "black":
                    self.send("playother\n")
        else:
            if self.feature_colors:
                if turn == "white":
                    self.send("white\n")
                elif turn == "black":
                    self.send("white\nblack\ngo\n")
            else:
                if turn == "white":
                    self.send("playother\n")
                elif turn == "black":
                    self.send("go\n")

    # Set the time and increment of the chess engine for the current match
    def set_time(self, time, inc):
        minutes = int(time) / 60
        seconds = int(time) % 60
        if seconds != 0:
            self.send("level 0 %d:%d %s\n" % (minutes, seconds, inc))
        else:
            self.send("level 0 %d %s\n" % (minutes, inc))

    # Sends a CECP message to the Chess Engine
    def send(self, msg):
        self.msg_queue.put(msg)

    # Receives a CECP message from the Chess Engine
    def recv(self, buffer_len=1024):
        try:
            msg = os.read(self.r_sock, buffer_len)
        except Exception, e:
            log("[pid %d] [ERROR] Unable to read from socket: '%s'" % (self.pid, e))
            return
        if len(msg) == 0:
            log("[pid %d] [ERROR] Connection closed unexpectedly" % (self.pid,))
            self.stop()
            return
        self.parser(msg)

    # Parse a CECP string, taking the needed actions according to it
    def parser(self, msg):
        cmds = msg.split("\n")
        for cmd in cmds:
            m = re.compile(r"move (\w+)").match(cmd)
            if m:
                move = m.group(1)
                # Enqueue the Chess Engine's move string
                self.w_bot.send(move)
                continue
            # Special move message from GNUChess
            m = re.compile(r"My move is: (\w+)").match(cmd)
            if m:
                move = m.group(1)
                # Enqueue the Chess Engine's move string
                self.w_bot.send(move)
                continue
            # Pong command, answer of a ping
            m = re.compile(r"pong (\d+)").match(cmd)
            if m:
                continue
            # Accepting a draw offer
            m = re.compile(r"offer draw").match(cmd)
            if m:
                self.accepted_draw = True
                continue
            # Declaration of supported features
            if re.compile(r"feature\s+").match(cmd):
                l1 = re.split(r"([^ \t\n\r\f\v=]+=(?:[^ \t\n\r\f\v=\"]+|\"[^=]+\"))", cmd)
                l2 = re.split(r"[^ \t\n\r\f\v=]+=(?:[^ \t\n\r\f\v=\"]+|\"[^=]+\")", cmd)
                for s in filter(lambda x:x not in l2, l1):
                    feature, value = s.split("=")
                    if feature == "done":
                        self.send("accepted done\n")
                        self.accepted_done = True
                    elif feature == "usermove":
                        if int(value) == 1:
                            self.feature_usermove = True
                        else:
                            self.feature_usermove = False
                        self.send("accepted usermove\n")
                    elif feature == "playother":
                        if int(value) == 1:
                            self.feature_colors = False
                        else:
                            self.feature_colors = True
                        self.send("accepted playother\n")
                    elif feature == "colors":
                        if int(value) == 1:
                            self.feature_colors = True
                        else:
                            self.feature_colors = False
                        self.send("accepted colors\n")
                    elif feature == "ping":
                        if int(value) == 1:
                            self.feature_ping = True
                        else:
                            self.feature_ping = False
                        self.send("accepted ping\n")
                    elif feature == "setboard":
                        if int(value) == 0:
                            log("[ERROR] Engine '%s' does not support 'setboard' command! Exiting..." % (self.path))
                            sys.exit(1)
                        self.send("accepted setboard\n")
                continue

if __name__ == '__main__':
    main()
