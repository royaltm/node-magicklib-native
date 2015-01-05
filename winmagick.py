import _winreg

def getmagickpath(regkey, regval):
  handle = _winreg.OpenKey(
      _winreg.HKEY_LOCAL_MACHINE,
      regkey,
      0,
      _winreg.KEY_READ | _winreg.KEY_WOW64_64KEY
    )
  return _winreg.QueryValueEx(handle, regval)[0]

print getmagickpath("SOFTWARE\\ImageMagick\\Current", "LibPath")
