Import('onb_env','pobj','ajobj','nsobj')

if not onb_env.has_key('_ALLJOYNCORE_'):
    onb_env.Append(LIBS = ['alljoyn'])
    if onb_env['BR'] == 'on' :
        onb_env['ajrlib'] = 'ajrouter'
    if onb_env['OS'] == 'openwrt':
        onb_env.AppendUnique(LIBS = [ 'stdc++', 'pthread' ])
    if onb_env['OS'] == 'linux':
        onb_env.AppendUnique(LIBS = [ 'rt', 'pthread' ])

## Make alljoyn_app dist a sub-directory of the alljoyn dist.  This avoids any conflicts with alljoyn dist targets.
## Link Device Mgmt 
onb_env['ALLJOYN_APP2_DISTDIR'] = onb_env['DISTDIR'] + '/cpp'
onb_env['PACKAGES'] = '#'

# librarie Alljoyn App
#onb_env.Install('$ALLJOYN_APP2_DISTDIR/../cpp/lib', onb_env.SConscript('../../../../../../../services/ajDev/SConscript', exports = ['onb_env']))

## build Alljoyn App programs
if onb_env['BUILD_SERVICES_SAMPLES'] == 'on':
    #onb_env.Install('$ALLJOYN_APP2_DISTDIR/bin', onb_env.SConscript('DeviceMgmt/SConscript', exports = ['onb_env','pobj','ajobj']))
    onb_env.Install(['$ALLJOYN_APP2_DISTDIR/bin','$PACKAGES/packages/App/bin'], onb_env.SConscript('DeviceMgmt/SConscript', exports =['onb_env','pobj','ajobj','nsobj']))

