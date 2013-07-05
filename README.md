# QML Tools

QML Tools provide a set of tools that help keeping the quality of large QML based applications high.

The first tool is QtCov, a combination of an instrumenter and a qml plugin for collecting runtime coverage data for QML applications.


## QtCov

QtCov is a coverage solution for the javascript code contained in QML files and their attached JavaScript files.

QtCov consist out of two parts, the QtCov QML Plugin and the instrumented task in the qml-tools binary.


### Instrumenting QML files

QtCov uses the private  Qt APIs in QQmlScript for parsing the QML files and analyzing the defined properties.

The list of properties containing JS bindings, signal handler and internal functions is then passed into istanbul ( http://gotwarlost.github.io/istanbul/ ) which creates the instrumented version of the source. 

All properties with bindings are instrumented, even the ones possibly using QV4Bindings or QCompiledBindings.

An internal mapper containing the coverage data is created, and injected to the header of the file.

Additionally a new QML plugin is added to the top of the QML file, this will be explained in the second part.

Also a base coverage_data.json file is generated that will be filled during runtime. The js-code lines/columns are mapped to QML file line/columns for this file. This is done to prevent skipping files that are missed during runtime (eg. not loaded or touched at all).

#### Example

```javascript
import QtQuick 2.0

Item {
    id: root
    width: 100
    height: 100
    
    property int value: { if (test > 5) { 1 } else { 2 } }
    property int test: 1
}
```

A simple QML file with one binding
   
```javascript   
{ if (test > 5) { 1 } else { 2 } }
```

Instrumenting this binding yields the following code

```javascript  
{__cov_1.s['1']++;if(test>5){__cov_1.b['1'][0]++;__cov_1.s['2']++;1;}else{__cov_1.b['1'][1]++;__cov_1.s['3']++;2;}}
```    

Also a new property is added

```javascript  
readonly property var __cov_1: QtCov.coverage.data['test/cases/Coverage.qml:8:25'] ? QtCov.coverage.data['test/cases/Coverage.qml:8:25'] : (QtCov.coverage.data['test/cases/Coverage.qml:8:25'] = {"path":"test/cases/Coverage.qml:8:25","s":{"1":0,"2":0,"3":0},"b":{"1":[0,0]},"f":{},"fnMap":{},"statementMap":{"1":{"start":{"line":8,"column":27},"end":{"line":8,"column":57}},"2":{"start":{"line":8,"column":43},"end":{"line":8,"column":45}},"3":{"start":{"line":8,"column":54},"end":{"line":8,"column":56}}},"branchMap":{"1":{"line":8,"type":"if","locations":[{"start":{"line":8,"column":27},"end":{"line":8,"column":27}},{"start":{"line":8,"column":27},"end":{"line":8,"column":27}}]}}});
```    
    
And a plugin is added at the top
    
    import QtCov 1.0 as QtCov
    
Leading to the following file

    import QtCov 1.0 as QtCov; import QtQuick 2.0

    Item {
        readonly property var __cov_1: QtCov.coverage.data['test/cases/Coverage.qml:8:25'] ? QtCov.coverage.data['test/cases/Coverage.qml:8:25'] : (QtCov.coverage.data['test/cases/Coverage.qml:8:25'] = {"path":"test/cases/Coverage.qml:8:25","s":{"1":0,"2":0,"3":0},"b":{"1":[0,0]},"f":{},"fnMap":{},"statementMap":{"1":{"start":{"line":8,"column":27},"end":{"line":8,"column":57}},"2":{"start":{"line":8,"column":43},"end":{"line":8,"column":45}},"3":{"start":{"line":8,"column":54},"end":{"line":8,"column":56}}},"branchMap":{"1":{"line":8,"type":"if","locations":[{"start":{"line":8,"column":27},"end":{"line":8,"column":27}},{"start":{"line":8,"column":27},"end":{"line":8,"column":27}}]}}});
        
    
        id: root
        width: 100
        height: 100
        
        property int value: {__cov_1.s['1']++;if(test>5){__cov_1.b['1'][0]++;__cov_1.s['2']++;1;}else{__cov_1.b['1'][1]++;__cov_1.s['3']++;2;}}
        property int test: 1
    }    
    
The **coverage_data.json** contains the same mapping

    {"path":"test/cases/Coverage.qml:8:25","s":{"1":0,"2":0,"3":0},"b":{"1":[0,0]},"f":{},"fnMap":{},"statementMap":{"1":{"start":{"line":8,"column":27},"end":{"line":8,"column":57}},"2":{"start":{"line":8,"column":43},"end":{"line":8,"column":45}},"3":{"start":{"line":8,"column":54},"end":{"line":8,"column":56}}},"branchMap":{"1":{"line":8,"type":"if","locations":[{"start":{"line":8,"column":27},"end":{"line":8,"column":27}},{"start":{"line":8,"column":27},"end":{"line":8,"column":27}}]}}}
        
When JS files get instrumented, similar things are happening.


    .pragma library
    
    
    print("test");
    
    
    function someCode() {
        print("bye");
    }
    
    
    function x() {
    	someCode();
    
    	if (a == v) {
    		return;
    	}
    }
    
    
    var a = 0;
    var v = 10;
    
    
Gets instrumented to

    .pragma library
    
    .import QtCov 1.0 as QtCov
    
    if (!QtCov.coverage.data['test/cases/Coverage.js']) {
       QtCov.coverage.data['test/cases/Coverage.js'] = {"path":"test/cases/Coverage.js","s":{"1":0,"2":0,"3":0,"4":0,"5":0,"6":0,"7":0,"8":0,"9":0},"b":{"1":[0,0]},"f":{"1":0,"2":0},"fnMap":{"1":{"name":"someCode","line":8,"loc":{"start":{"line":8,"column":0},"end":{"line":8,"column":20}}},"2":{"name":"x","line":13,"loc":{"start":{"line":13,"column":0},"end":{"line":13,"column":13}}}},"statementMap":{"1":{"start":{"line":5,"column":0},"end":{"line":5,"column":14}},"2":{"start":{"line":8,"column":0},"end":{"line":10,"column":1}},"3":{"start":{"line":9,"column":4},"end":{"line":9,"column":17}},"4":{"start":{"line":13,"column":0},"end":{"line":19,"column":1}},"5":{"start":{"line":14,"column":1},"end":{"line":14,"column":12}},"6":{"start":{"line":16,"column":1},"end":{"line":18,"column":2}},"7":{"start":{"line":17,"column":2},"end":{"line":17,"column":9}},"8":{"start":{"line":22,"column":0},"end":{"line":22,"column":10}},"9":{"start":{"line":23,"column":0},"end":{"line":23,"column":11}}},"branchMap":{"1":{"line":16,"type":"if","locations":[{"start":{"line":16,"column":1},"end":{"line":16,"column":1}},{"start":{"line":16,"column":1},"end":{"line":16,"column":1}}]}}};
    }
    var __cov_7 = QtCov.coverage.data['test/cases/Coverage.js'];
    
    __cov_7.s['1']++;print('test');__cov_7.s['2']++;function someCode(){__cov_7.f['1']++;__cov_7.s['3']++;print('bye');}__cov_7.s['4']++;function x(){__cov_7.f['2']++;__cov_7.s['5']++;someCode();__cov_7.s['6']++;if(a==v){__cov_7.b['1'][0]++;__cov_7.s['7']++;return;}else{__cov_7.b['1'][1]++;}}__cov_7.s['8']++;var a=0;__cov_7.s['9']++;var v=10;%      
                
    
### The QtCov QML plugin

To allow the coverage being modified at runtime a QML plugin was created. This allows storing the coverage data over a whole application session and it manages loading of previous coverage sessions and saving the data.

Currently only applications with single QQmlEngines are supported.

Whenever the plugin is loaded for the first time it loads and parses the **coverage_data.json** that was created by the instrumentation tool. 

This file can be located in several locations:

1. Defined by the environment variable QTCOV_COVERAGE_DATA_PATH
2. In the current directory
3. Next to the application executable

When the file is not found, the plugin will work nevertheless, and an updated version will be saved in the current directory when possible, but files not loaded in the tests will not be found in the newly created coverage file.

### Running the instrumented application

After the QML and JS files have been instrumented, the application can be run normally.

The initial coverage file changes from

```json
{"test/cases/Coverage.qml:8:25":{"path":"test/cases/Coverage.qml:8:25","s":{"1":0,"2":0,"3":0},"b":{"1":[0,0]},"f":{},"fnMap":{},"statementMap":{"1":{"start":{"line":8,"column":27},"end":{"line":8,"column":57}},"2":{"start":{"line":8,"column":43},"end":{"line":8,"column":45}},"3":{"start":{"line":8,"column":54},"end":{"line":8,"column":56}}},"branchMap":{"1":{"line":8,"type":"if","locations":[{"start":{"line":8,"column":27},"end":{"line":8,"column":27}},{"start":{"line":8,"column":27},"end":{"line":8,"column":27}}]}}}}
```

To

```json
{"test/cases/Coverage.qml:8:25":{"path":"test/cases/Coverage.qml:8:25","s":{"1":1,"2":0,"3":1},"b":{"1":[0,1]},"f":{},"fnMap":{},"statementMap":{"1":{"start":{"line":8,"column":27},"end":{"line":8,"column":57}},"2":{"start":{"line":8,"column":43},"end":{"line":8,"column":45}},"3":{"start":{"line":8,"column":54},"end":{"line":8,"column":56}}},"branchMap":{"1":{"line":8,"type":"if","locations":[{"start":{"line":8,"column":27},"end":{"line":8,"column":27}},{"start":{"line":8,"column":27},"end":{"line":8,"column":27}}]}}}}
```    
