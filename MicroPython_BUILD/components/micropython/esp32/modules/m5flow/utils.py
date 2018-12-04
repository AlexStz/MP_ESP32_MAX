import uos as os

# ---- Basic file func ----
def exists(fname):
    try:
        with open(fname):
            pass
        return True
    except OSError:
        return False

def isfile(fname):
    return exists(fname)

def isdir(path):
    try:
        os.listdir(path)
        return True
    except:
        return False

def filecp(sfile, pfile, blocksize=4096):
    with open(sfile, 'rb') as sf:
        with open(pfile, 'w+') as pf:
            while pf.write(sf.read(blocksize)):
                pass

def filesize(fname):
    if isfile(fname):
        return os.stat(fname)[6]
    else:
        return None

def makedirs(path):
    _path = ''
    if path[0] == '/':
        path = path.strip('/').split('/')
        for i in path:
            _path += '/' + i
            try:
                os.mkdir(_path)
            except:
                pass
    else:
        path = path.split('/')
        for i in path:
            _path += i
            try:
                os.mkdir(_path)
                _path += '/'
            except:
                pass

def lcd_text_button(number, message):
    x_pos = 0
    if number == 'A':
        x_pos = 62
    elif number == 'B':
        x_pos = 159
    elif number == 'C':
        x_pos = 253
    lcd.text(x_pos - int(lcd.textWidth(message) / 2), 235 - lcd.fontSize()[1], message)