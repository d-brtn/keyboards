module.exports = function (options, defines) {
  const customKeycodes = [
    {
      code: 'RHID_TOGG',
      name: 'R.HID\nTOGG',
      title: 'Toggle allow or deny access to RAW HID',
      shortName: 'R.HID\nTOGG'
    },
    {
      code: 'RHID_ON',
      name: 'R.HID\nON',
      title: 'Allow access to RAW HID',
      shortName: 'R.HID\nON'
    },
    {
      code: 'RHID_OFF',
      name: 'R.HID\nOFF',
      title: 'Deny access to RAW HID',
      shortName: 'R.HID\nOFF'
    },
    {
      code: 'MAC_TOGG',
      name: 'Mac\nTOGG',
      title: 'Toggle true apple mode with switching base layer 0(mac) or 1',
      shortName: 'Mac\nTOGG'
    },
    {
      code: 'MAC_ON',
      name: 'Mac\nON',
      title: 'Enable true apple mode with switching base layer 0',
      shortName: 'Mac\nON'
    },
    {
      code: 'MAC_OFF',
      name: 'Mac\nOFF',
      title: 'Enable true apple mode with switching base layer 1',
      shortName: 'Mac\nOFF'
    },
    {
      code: 'USJ_TOGG',
      name: 'USJ\nTOGG',
      title: 'Toggle enabling key overridng for ANSI layout on JIS environment',
      shortName: 'USJ\nTOGG'
    },
    {
      code: 'USJ_ON',
      name: 'USJ\nON',
      title: 'Enable key overriding for ANSI layout on JIS environment',
      shortName: 'USJ\nON'
    },
    {
      code: 'USJ_OFF',
      name: 'USJ\nOFF',
      title: 'Disable key overriding for ANSI layout on JIS environment',
      shortName: 'USJ\nOFF'
    },
    {
      code: 'APPLE_FN',
      name: 'Apple\nfn',
      title: 'Apple Fn/Globe Key',
      shortName: 'Apple\nfn'
    },
    {
      code: 'APPLE_FF',
      name: 'Apple\nfn+FK',
      title:
        "Apple Fn/Globe key for the keyboard that dosen't have F1-12 keys. F1-12 keys can be mapped on top row. When mac mode is off, It simulates mac fn functions.",
      shortName: 'Apple\nfn+FK'
    },
    {
      code: 'EISU_KANA',
      name: 'EISU\nKANA',
      title: 'Toggle send かな and 英数',
      shortName: 'EISU\nKANA'
    }
  ]
  if (options.RADIAL_CONTROLLER_ENABLE === 'yes') {
    Array.prototype.push.apply(customKeycodes, [
      {
        code: 'RC_BTN',
        name: 'RC\nBTN',
        title: 'The button located on radial controller',
        shortName: 'RC\nBTN'
      },
      {
        code: 'RC_CCW',
        name: 'RC\nCCW',
        title: 'Counter clockwise rotation of the radial controller',
        shortName: 'RC\nLeft'
      },
      {
        code: 'RC_CW',
        name: 'RC\nCW',
        title: 'Clockwise rotation of the radial controller',
        shortName: 'RC\nRight'
      },
      {
        code: 'RC_TUNE',
        name: 'RC\nTUNE',
        title: 'Dial rotation speed becomes slow',
        shortName: 'RC\nTUNE'
      }
    ])
  }
  return customKeycodes
}
