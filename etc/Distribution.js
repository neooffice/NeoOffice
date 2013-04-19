<script>
<![CDATA[

function runBashScriptAndSetResult(bashScript, volume) {
    if (bashScript == null)
        return true;

    var procResult;
    if (volume != null) {
        procResult = system.run('/bin/bash', '-x', '-c', bashScript, '/bin/bash', volume);
    }
    else {
        procResult = system.run('/bin/bash', '-x', '-c', bashScript, '/bin/bash');
    }

    if (procResult === 0) {
        return true;
    }
    else {
        my.result.type = 'Fatal';
        my.result.title = '';
        my.result.message = '';

        if (volume == null) {
            var title = system.localizedStandardStringWithFormat('InstallationCheckError', '$(PRODUCT_NAME_AND_VERSION)');
            if (title != null) {
                my.result.title = title.toString();
            }
        }

        var message = null;
        if (!isNaN(procResult)) {
            var key = procResult.toString();
            message = system.localizedString(key);
            if (message == key) {
                message = null;
            }
        }

        if (message == null && volume != null) {
            message = system.localizedStandardString('GENERIC_FAIL_VOLUME');
        }

        if (message != null) {
            my.result.message = message.toString();
        }

        return false;
    }
}

var installationCheckBashScript = null;

function runInstallationCheck() {
    var commandString = '$(REQUIRED_COMMANDS)';
    var commands = commandString.split(' ');
    for (var i = 0; i < commands.length; ++i) {
        var command = commands[i];
        if (command.length && !system.files.fileExistsAtPath('/usr/bin/' + command) && !system.files.fileExistsAtPath('/bin/' + command) && !system.files.fileExistsAtPath('/usr/sbin/' + command) && !system.files.fileExistsAtPath('/sbin/' + command)) {
            my.result.type = 'Fatal';
            my.result.title = '';
            my.result.message = '';

            var title = system.localizedStandardStringWithFormat('InstallationCheckError', '$(PRODUCT_NAME_AND_VERSION)');
            if (title != null) {
                my.result.title = title.toString();
            }

            var key = '16';
            var message = system.localizedString(key);
            if (message == key) {
                message = null;
            }

            if (message == null) {
                message = system.localizedStandardString('GENERIC_FAIL_VOLUME');
            }

            if (message != null) {
                my.result.message = message.toString();
            }

            return false;
        }
    }

    return true;
}

var volumeCheckBashScript = null;

function runVolumeCheck() {
    return runBashScriptAndSetResult(volumeCheckBashScript, my.target.mountpoint);
}

]]>
</script>

<installation-check script="runInstallationCheck();"/>

<volume-check script="runVolumeCheck();">
    <allowed-os-versions>
        <os-version min="$(PRODUCT_MIN_OSVERSION)"/>
    </allowed-os-versions>
</volume-check>
