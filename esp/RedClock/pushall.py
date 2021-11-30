# 
#    RedClock Copyright © 2021 Craige Hales
#    All rights reserved.
#
#    This source code is licensed under the BSD-style license found in the
#    LICENSE file in the root directory of this source tree. 
#
#    thanks to many people that posted bits of information!
#


import threading, time
import urllib.request, json, socket, base64
import curses
from curses import wrapper

def mainC(stdscr):
    stdscr.clear()
    curses.noecho()
    curses.cbreak()
    stdscr.keypad(True)
    curses.curs_set(False)

    # hunt for red clocks, apply updates

    # https://stackoverflow.com/questions/44239822/urllib-request-urlopenurl-with-authentication
    base64string = base64.b64encode(b'rc'+b':'+b'88888888').decode() # default basic auth password (decode bytes->str)
    
    gateway = "192.168.4."

    dir = "main/spiffsdata/"

    files = [ # size order, biggest uploads first
        "js/jquery-3.5.1.min.js",
        "js/masonry.pkgd.min.js",
        "favicon.ico",
        "css/gauge.min.css",
        "css/style.css",
        "index.html",
        "reboot.html",
        "about.html"  ]

    global nsearch

    global mutex
    mutex = threading.Lock()

    global statusrow
    statusrow=1 # only ~20 devices exist
    global firstdevrow
    firstdevrow=2+statusrow
    global errorrow
    errorrow=21+firstdevrow # there are 19 devices today, this allows for 19 error lines
#RC2       192.168.4.137   06:24:41 Aug  5 2021  RC2 TZ lookup built 20Mar2021:19:55:43 06:48:05 Aug  5 2021
#RC12      192.168.4.101   06:24:41 Aug  5 2021  RC12 TZ lookup built 20Mar2021:19:55:43 06:48:05 Aug  5 2021
    global namefield
    namefield = 0
    global ipfield
    ipfield = namefield+10 # 10 is RC8 length
    global oldDateStatusField
    oldDateStatusField = ipfield+16 # 16 is 192.168.4##.139 length
    global operationStatusField
    operationStatusField = oldDateStatusField+21 # 21 is 12:15:23 Jul 13 2021 length
    global filenameStatusField
    filenameStatusField = operationStatusField+20 # 20 is typ "uploading..." length. after that, a filename appears, sometimes

    def cursay(y,x,msg):
        mutex.acquire()
        stdscr.addstr(y, x, msg)
        stdscr.clrtoeol()
        stdscr.refresh()
        mutex.release()


    def deleteFile(idx,filename):
        global errorrow
        cursay(firstdevrow+idx, filenameStatusField, filename)

        data = urllib.parse.urlencode("").encode("utf-8")
        clockName = indexToClockName[idx]
        req = urllib.request.Request("http://"+clockNameToIP[clockName]+'/delete/'+filename, data=data)
        req.add_header("Authorization", "Basic %s" % base64string)
        try:
            with urllib.request.urlopen(req) as response:
                if response.code!=200:
                    print("response=",response,response.code)
                    the_page = response.read()
                    print("the page=",the_page)
                else:
                    the_page = response.read()
        except urllib.error.URLError as e:
#            print("error on file delete=",e)
            cursay(errorrow,0," err file del="+str(e));errorrow+=1
    def uploadFile(idx,srcfilename,dstfilename):
        global errorrow
        cursay(firstdevrow+idx, filenameStatusField, dstfilename)

        with open(srcfilename, 'rb') as reader:
            data = reader.read()
            clockName = indexToClockName[idx]
            req = urllib.request.Request("http://"+clockNameToIP[clockName]+'/upload/'+dstfilename, data=data)
            req.add_header("Authorization", "Basic %s" % base64string)
            try:
                with urllib.request.urlopen(req) as response:
                    if response.code!=200:
                        print("response=",response,response.code)
                        the_page = response.read()
                        print("the page=",the_page)
                    else:
                        the_page = response.read()
            except urllib.error.URLError as e:
#                print("error on file upload=",e)
                cursay(errorrow,0," err file upl="+str(e));errorrow+=1

    def reboot(ip):
        global errorrow
        req = urllib.request.Request("http://"+ip+"/reboot.cmd")
        req.add_header("Authorization", "Basic %s" % base64string)
        try:
            with urllib.request.urlopen(req,timeout=(1)) as url:
                print("attempt")
                print("unexpected: ",url.read())  # never get here?
        except socket.timeout as e: # 10/12 times this happens
#            cursay(errorrow,0,"ok reboot="+ip+" "+str(e));errorrow+=1
            pass
        except urllib.error.URLError as e: # 2/12 times this happens
#            cursay(errorrow,0,"ok reboot="+ip+" "+str(e));errorrow+=1
            pass
        except ConnectionResetError as e: # this happened, once
            cursay(errorrow,0,"ok reboot="+ip+" "+str(e));errorrow+=1
            pass

    def getStatus(ip,timeo):
        global errorrow
        for i in range(3):
            data1 = ""
            data2 = ""
            data3 = ""
            try:
                req = urllib.request.Request("http://"+ip+"/status.json")
                req.add_header("Authorization", "Basic %s" % base64string)
                with urllib.request.urlopen(req,timeout=(timeo)) as url:
                    data1 = url.read()
                    data2 = data1.decode()
                    data3 = json.loads(data2) 
                    return data3
            except socket.timeout as e:
#                print("get status socket.timeout",ip)
                cursay(errorrow,0," getstat="+ip+str(e));errorrow+=1
            except TimeoutError as e:
#                print("get status TimeoutError",ip)
                cursay(errorrow,0," getstat="+ip+str(e));errorrow+=1
            except urllib.error.URLError as e:
                pass
            except OSError as e:
#                print("get status OSError",ip,e)
                cursay(errorrow,0," getstat="+ip+str(e));errorrow+=1
            except UnicodeDecodeError as e:
                print(data1)
                cursay(errorrow,0,"UnicodeDecodeError"+e+data1);errorrow+=1
        return None

    def searcher(i,ntry,timeo): # run a bunch of these in parallel to look for IPs with clocks
        global errorrow
        global nsearch # init in search()
        global mutex
        for j in range(ntry):
            data = getStatus(gateway+str(i),timeo)
            if data is not None:
                ip = data['station_ip']
                clockName = data['access_point_name'] 
                
                ##print(clockName)
                #if clockName != "RC0": ### testing!
                #    continue
                    
                    
                clockNameToIP[clockName] = ip #print("station_ip=",data['station_ip']," access_point_name=",data['access_point_name'])

                mutex.acquire()
                firstTime = 0
                if clockName not in clockNameToIndex:
                    firstTime = 1
                    idx = len(clockNameToIndex)
                    indexToClockName[idx] = clockName # before adding to clockNameToIndex
                    clockNameToIndex[clockName] = idx
                mutex.release()

                if firstTime:

                    cursay(firstdevrow+idx, namefield, clockName)
                    cursay(firstdevrow+idx, ipfield, ip)

                    if 1:
                        data = getStatus(clockNameToIP[clockName],15)

                        cursay(firstdevrow+idx, oldDateStatusField, data['app_desc_time']+" "+data['app_desc_date'])

                        cursay(firstdevrow+idx, operationStatusField, "uploading exe...")

                        uploadFile(idx,"build/RedClock.bin", "Executable_Update.Bin")

                        cursay(firstdevrow+idx, operationStatusField, "uploading map...")

                        uploadFile(idx,"tzblob.bin","Timezone_Update.Bin")

                        cursay(firstdevrow+idx, operationStatusField, "deleting file...")

                        for filename in files: # delete all files first (spiffs space management?)
                            deleteFile(idx,filename)

                        cursay(firstdevrow+idx, operationStatusField, "uploading file...")

                        for filename in files: # upload all files, biggest first (spiffs?)
                            uploadFile(idx,dir+filename,filename)

                        cursay(firstdevrow+idx, operationStatusField, "rebooting (25 sec)...")

                        reboot(clockNameToIP[clockName])

                        # reboot in progress, typically 10 seconds
                        time.sleep(25) # make sure reboot is started?
                        # 5 seconds gets a really bad GPGLL message with a xB5 in the UTF-8 text that throws an error. try 25.
                        data = getStatus(clockNameToIP[clockName],15)
                        if clockName != data['access_point_name']:
                            errflag = " ☹" # is it possible for the ip to be lost during reboot?
                        else:
                            errflag = " ♕"
                        cursay(firstdevrow+idx, operationStatusField, " " +
                            data['serial_number'] + " " +
                            data['timezone_build_date'] + " " +
                            data['app_desc_time'] + " " +
                            data['app_desc_date'] + " " +
                            errflag+data['access_point_name']
                        )

                nsearch+=1
                break

    def search(ntry,timeo):
        global errorrow
        global nsearch
        nsearch = 0
        searchthread = {}
        for i in range(256):
            searchthread[i] = threading.Thread(target=searcher,args=(i,ntry,timeo,))
            searchthread[i].start()
        for i in range(256):
            searchthread[i].join(120) # sometimes it takes > 20 seconds

    # dictionaries...
    clockNameToIP = {}
    clockNameToIndex = {}
    indexToClockName = {}

    search(5,5)


    stdscr.addstr(statusrow, 0, "done, press key", curses.A_BOLD+curses.A_BLINK+curses.A_REVERSE)
    c = stdscr.getch()
    return


wrapper(mainC) # start the curses library
