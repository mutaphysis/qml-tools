import QtQuick 2.0

BaseElement {
    width: 100
    height: 62

    objectName: "baseName"

    property var inlineComponent: InlineElement { objectName: "inlineName"}
    property var boundComponent: testId
    property var emptyComponent: null

    anchors {}

    property int otherProperty: 4

    test: [InlineElement { objectName: "inlineName2" }, InlineElement { objectName: "inlineName3" }]

    ChildElement {
        id: testId
        objectName: "externalName"
        ChildSubElement {
        }
    }
}


// test
