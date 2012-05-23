<script>
<![CDATA[

function runBashScriptAndSetResult(bashScript, volume) {
    if (bashScript == null)
        return true;

    var procResult;
    if (volume != null) {
        procResult = system.run('/bin/bash', '-x', '-c', unescape(bashScript), '/bin/bash', volume);
    }
    else {
        procResult = system.run('/bin/bash', '-x', '-c', unescape(bashScript), '/bin/bash');
    }

    if (procResult === 0) {
        return true;
    }
    else {
        my.result.type = 'Fatal';
        my.result.message = null;
        if (!isNaN(procResult)) {
            // Temporary workaround to InstallationCheck exit values
            if (procResult > 96) {
                procResult -= 96;
            }

            var key = procResult.toString();
            my.result.message = system.localizedString(key);
            if (my.result.message == key) {
                my.result.message = null;
            }
        }
        if (my.result.message == null) {
            if (volume != null) {
                my.result.message = system.localizedStandardString('GENERIC_FAIL_VOLUME');
            }
            else {
                my.result.message = system.localizedStandardStringWithFormat('InstallationCheckError', '$(PRODUCT_NAME) $(PRODUCT_VERSION)');
            }
            if (my.result.message == null) {
                my.result.message = '';
            }
        }
        return false;
    }
}

function runInstallationCheck() {
    // Script must an escaped string with no newlines
    var installationCheckBashScript = null;
    return runBashScriptAndSetResult(installationCheckBashScript, null);
}

function runVolumeCheck() {
    // Script must an escaped string with no newlines
    var volumeCheckBashScript = null;
    return runBashScriptAndSetResult(volumeCheckBashScript, my.target.mountpoint);
}

]]>
</script>

<installation-check script="runInstallationCheck();"/>

<volume-check script="runVolumeCheck();"/>
