#!/usr/bin/python
#
#    IBM Omni driver
#    Copyright (c) International Business Machines Corp., 2000-2004
#
#    This library is free software; you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published
#    by the Free Software Foundation; either version 2.1 of the License, or
#    (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#    the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

import string
from xml.dom import Node
from xml.dom.ext import PrettyPrint

def firstNode (elm):
#   print "firstNode (", elm, ")"

    while     elm != None                          \
          and (  elm.nodeType == Node.TEXT_NODE    \
              or elm.nodeType == Node.COMMENT_NODE \
              ):
        elm = elm.nextSibling

#   print "firstNode returning", elm

    return elm

def nextNode (elm):
#   print "nextNode (", elm, ")"

    if elm != None:
        elm = elm.nextSibling

    while     elm != None                          \
          and (  elm.nodeType == Node.TEXT_NODE    \
              or elm.nodeType == Node.COMMENT_NODE \
              ):
        elm = elm.nextSibling

#   print "nextNode returning", elm

    return elm

def validSingleElement (elm):
    try:
        if len (elm.childNodes) != 1:
            return False

        elm = elm.firstChild

        if elm.nodeType != Node.TEXT_NODE:
            return False

        return True

    except Exception, e:
        return False

def getValue (elm):
    try:
        if len (elm.childNodes) == 1:
            elm = elm.firstChild

            return elm.nodeValue

    except Exception, e:
        pass

    return None

def getIntValue (elm):
    value = getValue (elm)
    if value == None:
        return value

    return 0

def convertXMLString (string):
    list = [ ('&quot;', '"'),
             ('&amp;',  '&'),
             ('&lt;',   '<'),
             ('&gt;',   '>'),
             ('&apos;', "'")
           ]

    for elm in list :
        (frm, to) = elm

        string = string.replace (frm, to)

    return string

def validHexChar (string):
    if string == "0":
        return 0
    elif string == "1":
        return 1
    elif string == "2":
        return 2
    elif string == "3":
        return 3
    elif string == "4":
        return 4
    elif string == "5":
        return 5
    elif string == "6":
        return 6
    elif string == "7":
        return 7
    elif string == "8":
        return 8
    elif string == "9":
        return 9
    elif string == "a":
        return 10
    elif string == "b":
        return 11
    elif string == "c":
        return 12
    elif string == "d":
        return 13
    elif string == "e":
        return 14
    elif string == "f":
        return 15
    elif string == "A":
        return 10
    elif string == "B":
        return 11
    elif string == "C":
        return 12
    elif string == "D":
        return 13
    elif string == "E":
        return 14
    elif string == "F":
        return 15
    else:
        return None

def parseHexGroup (string):
    if len (string) < 2:
        return (None, False)

    digitH = validHexChar (string[0])
    if digitH == None:
        return (None, False)

    digitL = validHexChar (string[1])
    if digitL == None:
        return (None, False)

    return (digitH * 16 + digitL, True)

def getCommand (elm):
    string = getValue (elm)
    return getCommandString (string)

def getCommandString (string):
    string         = convertXMLString (string)
    ptrString      = 0
    inString       = False
    char           = None
    nextChar       = None
    charFound      = False
    nextCharFound  = False
    command        = []
    shouldContinue = True

    try:
        while shouldContinue:
#           print "string[%d] = %s, charFound = %d, char = %s, nextCharFound = %d, nextChar = %s, inString = %d" % (ptrString, string[ptrString], charFound, char, nextCharFound, nextChar, inString)

            if nextCharFound:
                char          = nextChar
                nextCharFound = False
                charFound     = True

            else:
                if inString:
                    if "\\" == string[ptrString]:
                        pass
                    elif '"' == string[ptrString]:
                        ptrString += 1
                        inString   = False
                        continue
                    else:
                        char       = string[ptrString]
                        ptrString += 1
                        charFound  = True
                else:
                    while    string[ptrString] == "\r" \
                          or string[ptrString] == "\n" \
                          or string[ptrString] == " ":
                        ptrString += 1

                    if string[ptrString] == "_":
                        ptrString += 1

                        if string[ptrString:ptrString + 4] == "ESC_":
                            char      = "\x1b"
                            charFound = True
                            ptrString += 4

                        elif string[ptrString:ptrString + 4] == "NUL_":
                            char      = "\0"
                            charFound = True
                            ptrString += 4

                        elif string[ptrString:ptrString + 3] == "LF_":
                            char      = "\x0a"
                            charFound = True
                            ptrString += 3

                        elif string[ptrString:ptrString + 3] == "CR_":
                            char      = "\x0d"
                            charFound = True
                            ptrString += 3

                        elif string[ptrString:ptrString + 3] == "FF_":
                            char      = "\x0c"
                            charFound = True
                            ptrString += 3

                        else:
                            print "Error! Unknown define " + string
                            shouldContinue = False

                    elif string[ptrString] == "H":
                        ptrString += 1

                        if string[ptrString:ptrString + 4] == "EX2S":
                            ptrString += 4

                            while string[ptrString] == " ":
                                ptrString += 1

                            if string[ptrString] == "(":
                                ptrString += 1

                                while string[ptrString] == " ":
                                    ptrString += 1

                                (data, success) = parseHexGroup (string[ptrString:])

                                if success:
                                    ptrString += 2

                                    while string[ptrString] == " ":
                                        ptrString += 1

                                    if string[ptrString] == ",":
                                        ptrString += 1

                                        while string[ptrString] == " ":
                                            ptrString += 1

                                        (data2, success) = parseHexGroup (string[ptrString:])

                                        if success:
                                            ptrString += 2

                                            while string[ptrString] == " ":
                                                ptrString += 1

                                            if string[ptrString] == ")":
                                                ptrString += 1

                                                char           = unichr (data)
                                                nextChar       = unichr (data2)
                                                charFound      = True
                                                nextCharFound  = True

                                            else:
                                                print "Error: Expecting ')', found " + string[ptrString]
                                                shouldContinue = False

                                        else:
                                            print "Error: Couldn't parse hexgroup " + string[ptrString : ptrString + 2]
                                            shouldContinue = False

                                    else:
                                        print "Error: Expecting ',', found " + string[ptrString]
                                        shouldContinue = False

                                else:
                                    print "Error: Couldn't parse hexgroup " + string[ptrString : ptrString + 2]
                                    shouldContinue = False

                            else:
                                print "Error: Expecting '(', found " + string[ptrString]
                                shouldContinue = False

                        elif string[ptrString:ptrString + 2] == "EX":
                            ptrString += 2

                            while string[ptrString] == " ":
                                ptrString += 1

                            if string[ptrString] == "(":
                                ptrString += 1

                                while string[ptrString] == " ":
                                    ptrString += 1

                                (data, success) = parseHexGroup (string[ptrString:])

                                if success:
                                    ptrString += 2

                                    while string[ptrString] == " ":
                                        ptrString += 1

                                    if string[ptrString] == ")":
                                        ptrString += 1

                                        char      = unichr (data)
                                        charFound = True

                                    else:
                                        print "Error: Expecting ')', found " + string[ptrString]
                                        shouldContinue = False

                                else:
                                    print "Error: Couldn't parse hexgroup " + string[ptrString : ptrString + 2]
                                    shouldContinue = False

                            else:
                                print "Error: Expecting '(', found " + string[ptrString]
                                shouldContinue = False

                    elif string[ptrString] == "A":
                        ptrString += 1

                        if string[ptrString:ptrString + 4] == "SCII":
                            ptrString += 4

                            while string[ptrString] == " ":
                                ptrString += 1

                            if string[ptrString] == "(":
                                ptrString += 1

                                while string[ptrString] == " ":
                                    ptrString += 1

                                char      = string[ptrString]
                                charFound = True

                                while string[ptrString] == " ":
                                    ptrString += 1

                                if string[ptrString] == ")":
                                    ptrString += 1

                                else:
                                    print "Error: Expecting ')', found " + string[ptrString]
                                    shouldContinue = False

                            else:
                                print "Error: Expecting '(', found " + string[ptrString]
                                shouldContinue = False

                    elif string[ptrString] == '"':
                        ptrString += 1
                        inString   = True

            if charFound:
                command.append (char)

                charFound = False
    except Exception, e:
        pass

    return command

def encodeCommand (command):
    if len (command) == 0:
        return '""'

    encodedCommand = ""
    fInString      = False

    for cmd in command:
        if     cmd in string.printable \
           and not cmd in "\a\d\t":
            if not fInString:
                fInString       = True
                encodedCommand += '"'

            encodedCommand += cmd

        else:
            if fInString:
                encodedCommand += '" '
                fInString       = False

            if cmd == "\x1b":
                encodedCommand += "_ESC_ "
            elif cmd == "\0":
                encodedCommand += "_NUL_ "
            elif cmd == "\x0a":
                encodedCommand += "_LF_ "
            elif cmd == "\x0c":
                encodedCommand += "_FF_ "
            elif cmd == "\x0d":
                encodedCommand += "_CR_ "
            else:
                encodedCommand += "HEX (" + hex (ord (cmd))[2:].zfill (2) + ") "

    if fInString:
        encodedCommand += '"'

    return encodedCommand.strip ()

def countChildren (elm):
    elm   = firstNode (elm.firstChild)
    count = 0

    while elm != None:
        count += 1

        elm = nextNode (elm)

    return count

if __name__ == "__main__":
#   commands  = [ '_ESC_ &quot;~&quot; HEX(10) HEX (00) HEX (01) HEX (01) ' ]
    commands  = [ '&quot;!R! SPSZ 12; EXIT;&quot;',
                  '&quot;!R!PSRC1;EXIT;&quot;',
                  '&quot;!R!PSRC99;EXIT;&quot;',
                  '&quot;!R!spsz13;exit;&quot;',
                  '&quot;!R!spsz14;exit;&quot;',
                  '&quot;!R!spsz15;exit;&quot;',
                  '&quot;!R!spsz16;exit;&quot;',
                  '&quot;!R!spsz18;exit;&quot;',
                  '&quot;&quot;',
                  'HEX (01)',
                  'HEX (02)',
                  'HEX (03)',
                  'HEX (04)',
                  'HEX (05)',
                  'HEX (06)',
                  'HEX (07)',
                  'HEX (08)',
                  'HEX (09)',
                  '_ESC_  "&amp;l0M"',
                  '_ESC_  "&amp;l2M"',
                  '_ESC_  "&amp;l3M"',
                  '_ESC_  "&amp;l4M"',
                  '_ESC_  &quot;&amp;l-81&quot;',
                  '_ESC_  &quot;&amp;l-81A&quot;',
                  '_ESC_  &quot;&amp;l-90A&quot;',
                  '_ESC_  &quot;&amp;l109A&quot;',
                  '_ESC_  &quot;&amp;l15A&quot;',
                  '_ESC_  &quot;&amp;l16A&quot;',
                  '_ESC_  &quot;&amp;l1A&quot;',
                  '_ESC_  &quot;&amp;l1H&quot;',
                  '_ESC_  &quot;&amp;l25A&quot;',
                  '_ESC_  &quot;&amp;l26A&quot;',
                  '_ESC_  &quot;&amp;l27A&quot;',
                  '_ESC_  &quot;&amp;l29A&quot;',
                  '_ESC_  &quot;&amp;l2A&quot;',
                  '_ESC_  &quot;&amp;l2H&quot;',
                  '_ESC_  &quot;&amp;l3A&quot;',
                  '_ESC_  &quot;&amp;l3H&quot;',
                  '_ESC_  &quot;&amp;l45A&quot;',
                  '_ESC_  &quot;&amp;l46A&quot;',
                  '_ESC_  &quot;&amp;l6A&quot; _NUL_',
                  '_ESC_  &quot;&amp;l71A&quot;',
                  '_ESC_  &quot;&amp;l73A&quot;',
                  '_ESC_  &quot;&amp;l74A&quot;',
                  '_ESC_  &quot;&amp;l75A&quot;',
                  '_ESC_  &quot;&amp;l81A&quot;',
                  '_ESC_  &quot;&amp;l8H&quot;',
                  '_ESC_  &quot;&amp;l90A&quot;',
                  '_ESC_  &quot;&amp;l91A&quot;',
                  '_ESC_  &quot;&amp;l92A&quot;',
                  '_ESC_  &quot;&amp;l93A&quot;',
                  '_ESC_  &quot;&amp;l9H&quot;',
                  '_ESC_ &quot;&amp;l0H&quot;',
                  '_ESC_ &quot;&amp;l0M&quot;',
                  '_ESC_ &quot;&amp;l100A&quot;',
                  '_ESC_ &quot;&amp;l100H&quot;',
                  '_ESC_ &quot;&amp;l101A&quot;',
                  '_ESC_ &quot;&amp;l10H&quot;',
                  '_ESC_ &quot;&amp;l1A&quot;',
                  '_ESC_ &quot;&amp;l1H&quot;',
                  '_ESC_ &quot;&amp;l1J&quot;',
                  '_ESC_ &quot;&amp;l2000A&quot;',
                  '_ESC_ &quot;&amp;l2001A&quot;',
                  '_ESC_ &quot;&amp;l2007A&quot;',
                  '_ESC_ &quot;&amp;l2008A&quot;',
                  '_ESC_ &quot;&amp;l2009A&quot;',
                  '_ESC_ &quot;&amp;l2011A&quot;',
                  '_ESC_ &quot;&amp;l2012A&quot;',
                  '_ESC_ &quot;&amp;l2030A&quot;',
                  '_ESC_ &quot;&amp;l2032A&quot;',
                  '_ESC_ &quot;&amp;l2033A&quot;',
                  '_ESC_ &quot;&amp;l2034A&quot;',
                  '_ESC_ &quot;&amp;l2036A&quot;',
                  '_ESC_ &quot;&amp;l2037A&quot;',
                  '_ESC_ &quot;&amp;l2038A&quot;',
                  '_ESC_ &quot;&amp;l2039A&quot;',
                  '_ESC_ &quot;&amp;l2040A&quot;',
                  '_ESC_ &quot;&amp;l2041A&quot;',
                  '_ESC_ &quot;&amp;l25A&quot;',
                  '_ESC_ &quot;&amp;l26A&quot;',
                  '_ESC_ &quot;&amp;l26A&quot;_ESC_ &quot;&amp;l-190U&quot;',
                  '_ESC_ &quot;&amp;l27A&quot;',
                  '_ESC_ &quot;&amp;l28A&quot;',
                  '_ESC_ &quot;&amp;l2A&quot;',
                  '_ESC_ &quot;&amp;l2A&quot;_ESC_ &quot;&amp;l-230U&quot;',
                  '_ESC_ &quot;&amp;l2H&quot;',
                  '_ESC_ &quot;&amp;l30025A&quot;',
                  '_ESC_ &quot;&amp;l30092A&quot;',
                  '_ESC_ &quot;&amp;l37A&quot;',
                  '_ESC_ &quot;&amp;l3A&quot;',
                  '_ESC_ &quot;&amp;l3H&quot;',
                  '_ESC_ &quot;&amp;l45A&quot;',
                  '_ESC_ &quot;&amp;l46A&quot;',
                  '_ESC_ &quot;&amp;l4H&quot;',
                  '_ESC_ &quot;&amp;l5H&quot;',
                  '_ESC_ &quot;&amp;l60P&quot;',
                  '_ESC_ &quot;&amp;l65A&quot;',
                  '_ESC_ &quot;&amp;l66P&quot;',
                  '_ESC_ &quot;&amp;l6A&quot;',
                  '_ESC_ &quot;&amp;l6H&quot;',
                  '_ESC_ &quot;&amp;l70A&quot;',
                  '_ESC_ &quot;&amp;l70P&quot;',
                  '_ESC_ &quot;&amp;l71A&quot;',
                  '_ESC_ &quot;&amp;l72A&quot;',
                  '_ESC_ &quot;&amp;l7H&quot;',
                  '_ESC_ &quot;&amp;l80A&quot;',
                  '_ESC_ &quot;&amp;l81A&quot;',
                  '_ESC_ &quot;&amp;l84P&quot;',
                  '_ESC_ &quot;&amp;l89A&quot;',
                  '_ESC_ &quot;&amp;l8H&quot;',
                  '_ESC_ &quot;&amp;l90A&quot;',
                  '_ESC_ &quot;&amp;l91A&quot;',
                  '_ESC_ &quot;&amp;l9H&quot;',
                  '_ESC_ &quot;&amp;n10WdCardstock&quot;',
                  '_ESC_ &quot;&amp;n10WdTabstock2&quot;',
                  '_ESC_ &quot;&amp;n10WdTabstock3&quot;',
                  '_ESC_ &quot;&amp;n10WdTabstock4&quot;',
                  '_ESC_ &quot;&amp;n10WdTabstock5&quot;',
                  '_ESC_ &quot;&amp;n10WdTabstock6&quot;',
                  '_ESC_ &quot;&amp;n10WdTabstock7&quot;',
                  '_ESC_ &quot;&amp;n10WdTabstock8&quot;',
                  '_ESC_ &quot;&amp;n10WdTabstock9&quot;',
                  '_ESC_ &quot;&amp;n10WdThickBlue&quot;',
                  '_ESC_ &quot;&amp;n10WdThickGrey&quot;',
                  '_ESC_ &quot;&amp;n10WdThickNone&quot;',
                  '_ESC_ &quot;&amp;n10WdThickPink&quot;',
                  '_ESC_ &quot;&amp;n11WdLetterhead&quot;',
                  '_ESC_ &quot;&amp;n11WdPreprinted&quot;',
                  '_ESC_ &quot;&amp;n11WdPrepunched&quot;',
                  '_ESC_ &quot;&amp;n11WdSpecialRed&quot;',
                  '_ESC_ &quot;&amp;n11WdThickGreen&quot;',
                  '_ESC_ &quot;&amp;n11WdThickIvory&quot;',
                  '_ESC_ &quot;&amp;n12WdSpecialBlue&quot;',
                  '_ESC_ &quot;&amp;n12WdSpecialGrey&quot;',
                  '_ESC_ &quot;&amp;n12WdSpecialPink&quot;',
                  '_ESC_ &quot;&amp;n12WdThickOrange&quot;',
                  '_ESC_ &quot;&amp;n12WdThickPurple&quot;',
                  '_ESC_ &quot;&amp;n12WdThickYellow&quot;',
                  '_ESC_ &quot;&amp;n12WdTranslucent&quot;',
                  '_ESC_ &quot;&amp;n13WdSpecialGreen&quot;',
                  '_ESC_ &quot;&amp;n13WdSpecialIvory&quot;',
                  '_ESC_ &quot;&amp;n13WdTransparency&quot;',
                  '_ESC_ &quot;&amp;n14WdSpecialOrange&quot;',
                  '_ESC_ &quot;&amp;n14WdSpecialPurple&quot;',
                  '_ESC_ &quot;&amp;n14WdSpecialYellow&quot;',
                  '_ESC_ &quot;&amp;n15WdThickUserColor&quot;',
                  '_ESC_ &quot;&amp;n16WdThickLetterhead&quot;',
                  '_ESC_ &quot;&amp;n17WdSpecialUserColor&quot;',
                  '_ESC_ &quot;&amp;n18WdSpecialLetterhead&quot;',
                  '_ESC_ &quot;&amp;n5WdBond&quot;',
                  '_ESC_ &quot;&amp;n6WdColor&quot;',
                  '_ESC_ &quot;&amp;n6WdPlain&quot;',
                  '_ESC_ &quot;&amp;n7WdLabels&quot;',
                  '_ESC_ &quot;&amp;n7WdThick1&quot;',
                  '_ESC_ &quot;&amp;n7WdThick2&quot;',
                  '_ESC_ &quot;&amp;n7WdThick3&quot;',
                  '_ESC_ &quot;&amp;n8WdCustom1&quot;',
                  '_ESC_ &quot;&amp;n8WdCustom2&quot;',
                  '_ESC_ &quot;&amp;n8WdCustom3&quot;',
                  '_ESC_ &quot;&amp;n8WdCustom4&quot;',
                  '_ESC_ &quot;&amp;n8WdCustom5&quot;',
                  '_ESC_ &quot;&amp;n8WdSpecial&quot;',
                  '_ESC_ &quot;&amp;n9WdEnvelope&quot;',
                  '_ESC_ &quot;&amp;n9WdRecycled&quot;',
                  '_ESC_ &quot;&amp;n9WdTabstock&quot;',
                  '_ESC_ &quot;&amp;n9WdThickRed&quot;',
                  '_ESC_ &quot;(d&quot; HEX (02) _NUL_ HEX (01) &quot;h&quot;',
                  '_ESC_ &quot;(d&quot; HEX (02) _NUL_ _NUL_ &quot;Z&quot;',
                  '_ESC_ &quot;(d&quot; HEX (02) _NUL_ _NUL_ HEX (B4)',
                  '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (01) &quot;,&quot; HEX (01) &quot;,&quot;',
                  '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (01) &quot;h&quot; HEX (02) HEX (D0)',
                  '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (02) &quot;X&quot; HEX (02) &quot;X&quot;',
                  '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (02) &quot;X&quot; HEX (04) HEX (B0)',
                  '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (02) HEX (D0) HEX (02) HEX (D0)',
                  '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (02) HEX (D0) HEX (05) HEX (A0)',
                  '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(00) HEX(B4) HEX(00) HEX(B4)',
                  '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(01) HEX(2C) HEX(01) HEX(2C)',
                  '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(01) HEX(68) HEX(01) HEX(68)',
                  '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(02) HEX(58) HEX(02) HEX(58)',
                  '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(02) HEX(D0) HEX(02) HEX(D0)',
                  '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(02) HEX(D0) HEX(05) HEX(A0)',
                  '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (11)',
                  '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (12)',
                  '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (14)',
                  '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (15)',
                  '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (18)',
                  '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (19)',
                  '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (1F)',
                  '_ESC_ &quot;* %w&quot;',
                  '_ESC_ &quot;* &quot;',
                  '_ESC_ &quot;*!%w&quot;',
                  '_ESC_ &quot;*!&quot;',
                  '_ESC_ &quot;*%c%w&quot;',
                  '_ESC_ &quot;*&amp;%w&quot;',
                  '_ESC_ &quot;*&amp;&quot;',
                  '_ESC_ &quot;*&quot; HEX (02)',
                  '_ESC_ &quot;*&quot; HEX (02) &quot;%w&quot;',
                  '_ESC_ &quot;*&quot; HEX (03)',
                  '_ESC_ &quot;*&quot; HEX (03) &quot;%w&quot;',
                  '_ESC_ &quot;*&quot; HEX (04)',
                  '_ESC_ &quot;*&quot; HEX (04) &quot;%w&quot;',
                  '_ESC_ &quot;*&quot; HEX (05)',
                  '_ESC_ &quot;*&quot; HEX (05) &quot;%w&quot;',
                  '_ESC_ &quot;*&quot; HEX (06)',
                  '_ESC_ &quot;*&quot; HEX (06) &quot;%w&quot;',
                  '_ESC_ &quot;*&quot; HEX (07) &quot;%w&quot;',
                  '_ESC_ &quot;*&quot; _NUL_',
                  '_ESC_ &quot;*&quot; _NUL_ &quot;%w&quot;',
                  "_ESC_ &quot;*'%w&quot;",
                  '_ESC_ &quot;*(%w&quot;',
                  '_ESC_ &quot;*(&quot;',
                  '_ESC_ &quot;*@%w&quot;',
                  '_ESC_ &quot;*@&quot;',
                  '_ESC_ &quot;*A%w&quot;',
                  '_ESC_ &quot;*A&quot;',
                  '_ESC_ &quot;*F%w&quot;',
                  '_ESC_ &quot;*F&quot;',
                  '_ESC_ &quot;*G%w&quot;',
                  '_ESC_ &quot;*G&quot;',
                  '_ESC_ &quot;*I%w&quot;',
                  '_ESC_ &quot;*I&quot;',
                  '_ESC_ &quot;*g32W&lt;&quot; HEX (05) HEX (01) &quot;,&quot; HEX (01) &quot;,&quot; _NUL_ HEX (02) HEX (01) &quot;,&quot; HEX (01) &quot;,&quot; _NUL_ HEX (02) HEX (01) &quot;,&quot; HEX (01) &quot;,&quot; _NUL_ HEX (02) HEX (01) &quot;,&quot; HEX (01) &quot;,&quot; _NUL_ HEX (02) HEX (01) &quot;,&quot; HEX (01) &quot;,&quot; _NUL_ HEX (02)',
                  '_ESC_ &quot;*g32W&lt;&quot; HEX (05) HEX (02) &quot;X&quot; HEX (02) &quot;X&quot; _NUL_ HEX (02) HEX (02) &quot;X&quot; HEX (02) &quot;X&quot; _NUL_ HEX (02) HEX (02) &quot;X&quot; HEX (02) &quot;X&quot; _NUL_ HEX (02) HEX (02) &quot;X&quot; HEX (02) &quot;X&quot; _NUL_ HEX (02) HEX (02) &quot;X&quot; HEX (02) &quot;X&quot; _NUL_ HEX (02) _ESC_ &quot;*p2N&quot;',
                  '_ESC_ &quot;*o-1&quot;',
                  '_ESC_ &quot;*o0M&quot;',
                  '_ESC_ &quot;*o1M&quot;',
                  '_ESC_ &quot;*r1Q&quot;',
                  '_ESC_ &quot;*r2Q&quot;',
                  '_ESC_ &quot;*t100R&quot;',
                  '_ESC_ &quot;*t1200R&quot;',
                  '_ESC_ &quot;*t150R&quot;',
                  '_ESC_ &quot;*t200R&quot;',
                  '_ESC_ &quot;*t300R&quot;',
                  '_ESC_ &quot;*t400R&quot;',
                  '_ESC_ &quot;*t600R&quot;',
                  '_ESC_ &quot;*t75R&quot;',
                  '_ESC_ &quot;.%c%c%c%c%w&quot;',
                  '_ESC_ &quot;K%w&quot;',
                  '_ESC_ &quot;L%w&quot;',
                  '_ESC_ &quot;n&quot;',
                  '_ESC_ &quot;~&quot; HEX(10) HEX (00) HEX (01) HEX (01) ',
                  '_ESC_ &quot;~&quot; HEX(10) HEX (00) HEX (01) HEX (02) ',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) &quot;,K?&quot;HEX(8F) HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) &quot;-&quot; HEX(99) &quot;;&quot; HEX(A9) HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) &quot;-&quot; HEX(99) &quot;L&quot; HEX(89) HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) &quot;6&quot; HEX(B3) &quot;Ne&quot; HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) &quot;?&quot; HEX (8F) &quot;Z&quot; HEX(CC) HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) HEX (13) HEX(EE) HEX (1E) HEX(90) HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) HEX(1E) HEX(8F) &quot;,K&quot; HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) HEX(26) HEX(17) HEX(36) HEX(B3) HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1)  &quot;&amp;&quot; HEX (88) &quot;7$&quot; HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1)  &quot;&lt;&quot; HEX (1A) &quot;]&quot; HEX (DA) HEX (02)',
                  "_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1)  &quot;'&quot; HEX (02) &quot;9J&quot; HEX (02)",
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1) &quot;,&quot; HEX (BC) &quot;@&quot; _NUL_ HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1) &quot;.&quot; HEX (A0)  &quot;&lt;&quot; HEX (1A) HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1) &quot;.&quot; HEX (A0) &quot;L&quot; HEX (FA) HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1) HEX (14) &quot;_&quot; HEX (1F) HEX (01) HEX (02)',
                  '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1) HEX (1F) HEX (01) &quot;,&quot; HEX (BB) HEX (02)',
                  '_ESC_ &quot;~F&quot; _NUL_ HEX (05) _NUL_ _NUL_ HEX (01) _NUL_ _NUL_',
                  '_ESC_ &quot;~F&quot; _NUL_ HEX (05) _NUL_ _NUL_ HEX (02) _NUL_ _NUL_',
                  '_ESC_ &quot;~F&quot; _NUL_ HEX (05) _NUL_ _NUL_ HEX (03) _NUL_ _NUL_',
                  '_ESC_ &quot;~F&quot; _NUL_ HEX (05) _NUL_ _NUL_ HEX (FE) _NUL_ _NUL_',
                  '_ESC_ &quot;~F&quot; _NUL_ HEX (05) _NUL_ _NUL_ _NUL_ _NUL_ _NUL_',
                  '_ESC_ HEX (19) &quot;0&quot;',
                  '_ESC_ HEX (19) &quot;1&quot;',
                  '_ESC_ HEX (19) &quot;2&quot;',
                  '_ESC_ HEX (19) &quot;?&quot;',
                  '_ESC_ HEX (19) &quot;B&quot;',
                  '_ESC_ HEX (19) &quot;F&quot;',
                  '_NUL_'
                ]

    for command in commands:
        cmd        = getCommandString (command)
        encodedCmd = encodeCommand (cmd)
        cmd2       = getCommandString (encodedCmd)
        print cmd, '<-', encodedCmd

        if cmd != cmd2:
            print "Error: Commands are not equal"
            raise SystemExit

def XMLHeader ():
    return """<?xml version="1.0" encoding="UTF-8"?>
<!--
     IBM Omni driver
     Copyright (c) International Business Machines Corp., 2000-2004

     This library is free software; you can redistribute it and/or modify
     it under the terms of the GNU Lesser General Public License as published
     by the Free Software Foundation; either version 2.1 of the License, or
     (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
     the GNU Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public License
     along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
-->"""

