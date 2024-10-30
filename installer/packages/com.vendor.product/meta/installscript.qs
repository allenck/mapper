function Component()
{
    installer.setValue("ProductUUID", "{d26face9-b0a4-4121-8144-3dedc1f279e3}");

    var previous = installer.value("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{d26face9-b0a4-4121-8144-3dedc1f279e3}\\InstallLocation");
    if(previous !== "")
    {
        installer.setValue("TargetDir", previous);
        installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
    }
    installer.installationFinished.connect(this, Component.prototype.installationFinishedPageIsShown);
    installer.finishButtonClicked.connect(this, Component.prototype.installationFinished);
}

Component.prototype.createOperationsForArchive = function(archive)
{
    component.createOperationsForArchive(archive);
    console.log("creating start menu entries");
    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/README.txt", "@StartMenuDir@/README.lnk",
            "workingDirectory=@TargetDir@", "iconPath=@TargetDIR@/tram-icon.ico",
            "iconId=1", "description=Open README file");
        component.addOperation("CreateShortcut", "@TargetDir@/mapper.exe", "@StartMenuDir@/mapper.lnk",
            "workingDirectory=@TargetDir@");
        component.addOperation("CreateShortcut", "@TargetDir@/maintenancetool.exe", "@StartMenuDir@/maintenancetool.lnk",
            "workingDirectory=@TargetDir@", "iconPath=@TargetDIR@/tram-icon.ico",
            "iconId=2", "description=Maintenance Tool");
        }
}

Component.prototype.installationFinishedPageIsShown = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
            installer.addWizardPageItem( component, "ReadMeCheckBoxForm", QInstaller.InstallationFinished );
        }
    } catch(e) {
        console.log(e);
    }
}

Component.prototype.installationFinished = function()
{
    // add an operation to install redistributable
    component.addOperation("Execute", "@TargetDir@/vcredist_x64.exe", "/quiet", "/norestart");

    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
            var checkboxForm = component.userInterface( "ReadMeCheckBoxForm" );
            if (checkboxForm && checkboxForm.readMeCheckBox.checked) {
                QDesktopServices.openUrl("file:///" + installer.value("TargetDir") + "/README.txt");
            }
        }
    } catch(e) {
        console.log(e);
    }
}
