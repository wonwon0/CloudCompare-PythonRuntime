from typing import Union, List

class ccPythonInstance:

    def getMainWindow() -> QMainWindow: ...

    def getActiveGLWindow() -> ccGLWindow: ...

    def haveSelection(self) -> bool: ...

    def haveOneSelection(self) -> bool: ...

    def getSelectedEntities(self) -> List[Union[ccHObject, ccPointCloud, ccMesh]]: ...

    def addToDB(self, obj: ccHObject, updateZoom: bool = False, autoExpandDBTree: bool = True, checkDimensions: bool = False, autoRedeaw: bool = True) -> None: ...

    def redrawAll(only2D: bool = False) -> None: ...

    def refreshAll(only2D: bool = False) -> None: ...

    def disableAll() -> None: ...

    def updateUI() -> None: ...

    def freezeUI(state: bool) -> None: ...

def GetInstance() -> ccPythonInstance: ...
