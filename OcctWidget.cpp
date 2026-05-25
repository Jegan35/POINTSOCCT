// ==========================================
// 1. ALL QT HEADERS MUST BE INCLUDED FIRST
// ==========================================
#include <QMouseEvent>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <Graphic3d_MaterialAspect.hxx> // For metallic rendering
#include <QTimer>
#include <Graphic3d_HorizontalTextAlignment.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>


// ==========================================
// 2. LOCAL HEADER INCLUDED SECOND
// ==========================================
#include "OcctWidget.h"

// ==========================================
// 3. OPENCASCADE & X11 HEADERS INCLUDED LAST
// ==========================================
#if defined(Q_OS_WIN)
#include <WNT_Window.hxx>
#elif defined(Q_OS_MAC)
#include <Cocoa_Window.hxx>
#else
#include <Xw_Window.hxx>
#include <X11/Xlib.h>
#endif

#include <Aspect_DisplayConnection.hxx>
#include <AIS_Shape.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <gp_Pnt.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <Geom_Axis2Placement.hxx>
#include <AIS_Trihedron.hxx>
#include <StlAPI_Reader.hxx>
#include <RWStl.hxx>
#include <Poly_Triangulation.hxx>
#include <AIS_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <AIS_ViewCube.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <AIS_TextLabel.hxx>
#include <TCollection_ExtendedString.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <XCAFApp_Application.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDF_LabelSequence.hxx>

// ✅ ADD THE MISSING GRID HEADERS HERE:
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <cmath>


OcctWidget::OcctWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_DontCreateNativeAncestors); // ✅ MUST BE HERE

    setMouseTracking(true);
    myCSVPath = "/home/texsonics/Videos/extracted_paths.csv";
}
OcctWidget::~OcctWidget() {}


// =================================================================
// CREATES THICK 3D ARROWS (ULTRA-LIGHTWEIGHT PERFORMANCE)
// =================================================================
Handle(AIS_ColoredShape) OcctWidget::createThickTriad(double scale)
{
    TopoDS_Compound triad;
    BRep_Builder builder;
    builder.MakeCompound(triad);

    double cylR = 4.0 * scale;   // Slightly adjusted for better visuals
    double cylL = 80.0 * scale;
    double coneR = 10.0 * scale;
    double coneL = 20.0 * scale;

    // X Axis (Red)
    TopoDS_Shape xCyl = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,0,0), gp_Dir(1,0,0)), cylR, cylL).Shape();
    TopoDS_Shape xCone = BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(cylL,0,0), gp_Dir(1,0,0)), coneR, 0, coneL).Shape();

    // Y Axis (Green)
    TopoDS_Shape yCyl = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,0,0), gp_Dir(0,1,0)), cylR, cylL).Shape();
    TopoDS_Shape yCone = BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(0,cylL,0), gp_Dir(0,1,0)), coneR, 0, coneL).Shape();

    // Z Axis (Blue)
    TopoDS_Shape zCyl = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,0,0), gp_Dir(0,0,1)), cylR, cylL).Shape();
    TopoDS_Shape zCone = BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(0,0,cylL), gp_Dir(0,0,1)), coneR, 0, coneL).Shape();

    builder.Add(triad, xCyl); builder.Add(triad, xCone);
    builder.Add(triad, yCyl); builder.Add(triad, yCone);
    builder.Add(triad, zCyl); builder.Add(triad, zCone);

    // 🚀 CRITICAL OPTIMIZATION: Force a coarse, lightweight mesh! (5.0 instead of 0.5)
    BRepMesh_IncrementalMesh(triad, 5.0);

    Handle(AIS_ColoredShape) coloredTriad = new AIS_ColoredShape(triad);

    // Turn off heavy edge calculations
    coloredTriad->Attributes()->SetFaceBoundaryDraw(Standard_False);
    coloredTriad->Attributes()->SetIsoOnTriangulation(Standard_False);

    // Apply colors
    coloredTriad->SetCustomColor(xCyl, Quantity_NOC_RED);
    coloredTriad->SetCustomColor(xCone, Quantity_NOC_RED);
    coloredTriad->SetCustomColor(yCyl, Quantity_NOC_GREEN);
    coloredTriad->SetCustomColor(yCone, Quantity_NOC_GREEN);
    coloredTriad->SetCustomColor(zCyl, Quantity_NOC_BLUE1);
    coloredTriad->SetCustomColor(zCone, Quantity_NOC_BLUE1);

    return coloredTriad;
}

void OcctWidget::initOCCT()
{
    if (!myView.IsNull()) return;

    Handle(Aspect_DisplayConnection) displayConnection = new Aspect_DisplayConnection();
    myGraphicDriver = new OpenGl_GraphicDriver(displayConnection);

    myViewer = new V3d_Viewer(myGraphicDriver);
    myViewer->SetDefaultLights();
    myViewer->SetLightOn();

    myView = myViewer->CreateView();

#if defined(Q_OS_WIN)
    Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle)winId());
#elif defined(Q_OS_MAC)
    Handle(Cocoa_Window) wind = new Cocoa_Window((NSView *)winId());
#else
    Handle(Xw_Window) wind = new Xw_Window(displayConnection, static_cast<Window>(winId()));
#endif

    myView->SetWindow(wind);
    if (!wind->IsMapped()) {
        wind->Map();
    }

    // ==========================================
    // ✅ FIX 1: INDEPENDENT BACKGROUND COLORS
    // ==========================================
    if (myRole == MainRole) {
        // Left Panel gets the bright, professional gradient
        myView->SetBgGradientColors(Quantity_NOC_WHITE, Quantity_NOC_GRAY90, Aspect_GFM_VER);
    } else {
        // Right Panel gets the pure Black Screen Box!
        myView->SetBackgroundColor(Quantity_NOC_BLACK);
    }

    myContext = new AIS_InteractiveContext(myViewer);

    // Hardware Anti-Aliasing for smooth lines
    myView->ChangeRenderingParams().IsAntialiasingEnabled = Standard_True;
    myView->ChangeRenderingParams().NbMsaaSamples = 8;

    // ==========================================
    // ✅ FIX 2: ISOLATE THE GRID AND VIEWCUBE
    // ==========================================
    if (myRole == MainRole) {
        Handle(AIS_ViewCube) viewCube = new AIS_ViewCube();

        viewCube->SetDrawAxes(Standard_True);
        viewCube->SetSize(55);
        viewCube->SetFontHeight(12);
        viewCube->SetAxesLabels("X", "Y", "Z");

        Handle(Graphic3d_TransformPers) trsfPers = new Graphic3d_TransformPers(
            Graphic3d_TMF_TriedronPers,
            Aspect_TOTP_RIGHT_UPPER,
            Graphic3d_Vec2i(85, 85)
            );
        viewCube->SetTransformPersistence(trsfPers);
        myContext->Display(viewCube, Standard_False);

        // ✅ ONLY draw the giant 4000x4000 room grid if this is the Main Left Panel!
        // This stops the Right Panel from doing a massive "FitAll" zoom jerk!
        drawRoomGrid();
    }
}

void OcctWidget::loadStepFile(const std::string& filePath)
{
    if (myView.IsNull()) initOCCT();
    clearSelections();

    // 1. Initialize XDE Document (Required for CAF)
    Handle(TDocStd_Document) aDoc;
    Handle(XCAFApp_Application) anApp = XCAFApp_Application::GetApplication();
    anApp->NewDocument("MDTV-XCAF", aDoc);

    // 2. Read the file
    STEPCAFControl_Reader reader;
    reader.SetColorMode(Standard_True); // Ensure colors are imported
    reader.SetNameMode(Standard_True);  // Ensure names are imported

    IFSelect_ReturnStatus stat = reader.ReadFile(filePath.c_str());

    if (stat == IFSelect_RetDone) {
        reader.Transfer(aDoc);

        // 3. Get the top-level shape tool
        Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
        TDF_LabelSequence labels;
        aShapeTool->GetFreeShapes(labels);
        if (labels.Length() > 0) {
            TDF_Label aLabel = labels.Value(1);

            // 4. Create the XCAFPrs_AISObject (This maps colors/names automatically)
            myLoadedPart = new XCAFPrs_AISObject(aLabel);

            // ✅ THE FIX: Force the 3D model to be Solid/Shaded (Mode 1)!
            // Without this line, OCCT defaults to Wireframe/Transparent (Mode 0).
            myContext->SetDisplayMode(myLoadedPart, 1, Standard_False);

            // 5. Display the object
            myContext->Display(myLoadedPart, Standard_True);

            // 6. Setup your origin and positioning
            myCustomOrigin = gp_Pnt(0.0, 0.0, 0.0);
            offsetWorkpiece(.0, -800.0, 0.0);

            // Only fit the camera if this is the SideRole (DXF view)
            if (myRole == OcctWidget::SideRole) {
                myView->FitAll();
                myView->Redraw();
            } else {
                myView->Redraw(); // Ensure the main view repaints!
            }

            setSelectionMode(myCurrentSelectionMode);

            emit statusUpdate("✅ Workpiece Loaded (Solid & Colored).");
        }
    } else {
        emit statusUpdate("❌ Error: Failed to load STEP file via XDE.");
    }
}
void OcctWidget::resetOrigin()
{
    if (myOriginMarker.IsNull()) {
        emit statusUpdate("⚠️ Warning: No model loaded to reset.");
        return;
    }

    if (QMessageBox::question(this, "Reset Origin", "Are you sure you want to reset the origin back to the default Center of Mass?") == QMessageBox::Yes) {
        myCustomOrigin = myDefaultOrigin;

        gp_Ax2 defaultCoords(myCustomOrigin, gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
        Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(defaultCoords);
        myOriginMarker->SetComponent(placement);
        myContext->Redisplay(myOriginMarker, Standard_True);

        emit statusUpdate("🔄 Origin Reset to Default Center of Mass. Note: CSV data will use this new origin on your next Path action.");
    }
}

void OcctWidget::setSelectionMode(int mode)
{
    if(myContext.IsNull()) return;

    myCurrentSelectionMode = mode;

    // 1. Deactivate everything (This accidentally puts the ViewCube to sleep!)
    myContext->Deactivate();

    // 2. Turn on your new Selection rules for the robot parts
    switch(mode) {
    case 1: myContext->Activate(AIS_Shape::SelectionMode(TopAbs_FACE)); break;
    case 2: myContext->Activate(AIS_Shape::SelectionMode(TopAbs_EDGE)); break;
    case 3: myContext->Activate(AIS_Shape::SelectionMode(TopAbs_WIRE)); break;
    default: myContext->Activate(0);
    }

    // ==========================================
    // ✅ THE FIX: WAKE THE VIEW CUBE BACK UP!
    // ==========================================
    // We search the screen for the ViewCube and force its interactivity back on.
    AIS_ListOfInteractive displayedObjects;
    myContext->DisplayedObjects(displayedObjects);
    for (const Handle(AIS_InteractiveObject)& obj : displayedObjects) {
        if (obj->DynamicType() == STANDARD_TYPE(AIS_ViewCube)) {
            myContext->Activate(obj, 0); // 0 is the default click mode
        }
    }
}
void OcctWidget::enableOriginSelectionMode()
{
    myIsSettingOriginMode = true;
    emit statusUpdate("🎯 Origin Mode ACTIVE: Click any edge, face, or wire on the 3D model to snap the origin to it.");
}

// ====================================================================
// NEW CORE ARCHITECTURE: History, Undo, Redo & CSV Generation
// ====================================================================

void OcctWidget::clearSelections()
{
    // Remove all red lines from the screen
    for (const auto& step : myPathHistory) myContext->Remove(step.visualRedPath, Standard_False);
    for (const auto& step : myRedoStack) myContext->Remove(step.visualRedPath, Standard_False);

    myPathHistory.clear();
    myRedoStack.clear();
    myContext->UpdateCurrentViewer();

    regenerateCSV(); // This will clear the file
    emit statusUpdate("❌ All selections cleared. CSV wiped.");
}

void OcctWidget::undoSelection()
{
    if (myPathHistory.empty()) {
        emit statusUpdate("⚠️ Nothing to undo.");
        return;
    }

    // Move from Active History to Redo Stack
    PathData lastAction = myPathHistory.back();
    myPathHistory.pop_back();
    myContext->Remove(lastAction.visualRedPath, Standard_True); // Hide the red line
    myRedoStack.push_back(lastAction);

    regenerateCSV();
    emit statusUpdate(QString("↩️ Undo successful. Current Paths in CSV: %1").arg(myPathHistory.size()));
}

void OcctWidget::redoSelection()
{
    if (myRedoStack.empty()) {
        emit statusUpdate("⚠️ Nothing to redo.");
        return;
    }

    // Move from Redo Stack back to Active History
    PathData nextAction = myRedoStack.back();
    myRedoStack.pop_back();
    myContext->Display(nextAction.visualRedPath, Standard_True); // Show the red line again
    myPathHistory.push_back(nextAction);

    regenerateCSV();
    emit statusUpdate(QString("↪️ Redo successful. Current Paths in CSV: %1").arg(myPathHistory.size()));
}

// Update the function signature to take the parameter
void OcctWidget::processCurrentSelection(double resolution)
{
    if (myContext.IsNull() || !myContext->HasSelectedShape()) return;

    // ✅ THE FIX: Clear previous red lines so ONLY the new selection is red!
    // (If you ever DO want to select multiple, just hold the SHIFT key on your keyboard)
    if (!(QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
        clearSelections();
    }

    myContext->InitSelected();
    int addedCount = 0;

    QString xyzData;
    QTextStream stringOut(&xyzData);
    stringOut << "X,Y,Z\n";

    while (myContext->MoreSelected()) {
        TopoDS_Shape shape = myContext->SelectedShape();

        Handle(AIS_Shape) plottedPath = new AIS_Shape(shape);
        myContext->SetColor(plottedPath, Quantity_NOC_RED, Standard_False);

        if (shape.ShapeType() == TopAbs_FACE) {
            myContext->SetDisplayMode(plottedPath, 1, Standard_False);
        } else {
            myContext->SetWidth(plottedPath, 3.0, Standard_False);
        }

        myContext->Display(plottedPath, Standard_True);

        // Save the click to History
        myPathHistory.push_back({shape, plottedPath, resolution});
        addedCount++;

        // Run your math
        switch (shape.ShapeType()) {
        case TopAbs_FACE: processFace(TopoDS::Face(shape), stringOut, resolution); break;
        case TopAbs_WIRE: processWire(TopoDS::Wire(shape), stringOut, resolution); break;
        case TopAbs_EDGE: processEdge(TopoDS::Edge(shape), stringOut, resolution); break;
        default: break;
        }

        myContext->NextSelected();
    }
    myContext->ClearSelected(Standard_True);
    myRedoStack.clear();

    emit coordinatesExtracted(xyzData);
    regenerateCSV();
    emit statusUpdate(QString("✅ Extracted %1 new path(s). Total Paths in CSV: %2").arg(addedCount).arg(myPathHistory.size()));

    // ✅ ADD THIS LINE: Tells the UI to disable the "GET POINTS" button again
    emit selectionChanged(false);
}

void OcctWidget::regenerateCSV()
{
    QFile file(myCSVPath);
    // WriteOnly + Truncate means it completely overwrites the old file instantly
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << myCSVPath;
        return;
    }

    QTextStream out(&file);
    out << "X,Y,Z\n";

    // Play back the entire history stack to generate the perfect CSV state
    for (const auto& step : myPathHistory) {
        switch (step.shape.ShapeType()) {
        case TopAbs_FACE: processFace(TopoDS::Face(step.shape), out, step.resolution); break;
        case TopAbs_WIRE: processWire(TopoDS::Wire(step.shape), out, step.resolution); break;
        case TopAbs_EDGE: processEdge(TopoDS::Edge(step.shape), out, step.resolution); break;
        default: break;
        }
    }
    file.close();
}

// ====================================================================

void OcctWidget::processFace(const TopoDS_Face& face, QTextStream& out, double resolution)
{
    TopExp_Explorer wireExplorer(face, TopAbs_WIRE);
    int loopCount = 1;
    for (; wireExplorer.More(); wireExplorer.Next()) {
        TopoDS_Wire wire = TopoDS::Wire(wireExplorer.Current());
        QString boundaryMarker = QString("--- NEW BOUNDARY LOOP %1 ---").arg(loopCount);
        out << boundaryMarker << "\n";
        processWire(wire, out, resolution);
        loopCount++;
    }
}

void OcctWidget::processWire(const TopoDS_Wire& wire, QTextStream& out, double resolution)
{
    BRepAdaptor_CompCurve compCurve(wire, Standard_True);
    Standard_Real first = compCurve.FirstParameter();
    Standard_Real last = compCurve.LastParameter();
    GCPnts_UniformAbscissa discretizer(compCurve, resolution, first, last);

    if (discretizer.IsDone()) {
        for (int i = 1; i <= discretizer.NbPoints(); ++i) {
            Standard_Real param = discretizer.Parameter(i);
            gp_Pnt pt = compCurve.Value(param);

            double localX = pt.X() - myCustomOrigin.X();
            double localY = pt.Y() - myCustomOrigin.Y();
            double localZ = pt.Z() - myCustomOrigin.Z();
            out << localX << "," << localY << "," << localZ << "\n";
        }
    }
}

void OcctWidget::processEdge(const TopoDS_Edge& edge, QTextStream& out, double resolution)
{
    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
    if (curve.IsNull()) return;

    BRepAdaptor_Curve adaptor(edge);
    GCPnts_UniformAbscissa discretizer(adaptor, resolution, first, last);

    if (discretizer.IsDone()) {
        for (int i = 1; i <= discretizer.NbPoints(); ++i) {
            Standard_Real param = discretizer.Parameter(i);
            gp_Pnt pt = adaptor.Value(param);

            double localX = pt.X() - myCustomOrigin.X();
            double localY = pt.Y() - myCustomOrigin.Y();
            double localZ = pt.Z() - myCustomOrigin.Z();
            out << localX << "," << localY << "," << localZ << "\n";
        }
    }
}

void OcctWidget::paintEvent(QPaintEvent *event)
{
    // ✅ MUST BE CALLED: Keeps Qt's internal rendering loop happy
    QWidget::paintEvent(event);

    if (myView.IsNull()) initOCCT();
    myView->Redraw();
}

void OcctWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!myView.IsNull()) {
        myView->MustBeResized(); // Tell X11 to update the dimensions

        // ✅ NEW: Automatically re-frame the camera when the layout updates!
        // This ensures the grid and text are never cut off after the "MAX" fix.
        myView->FitAll();

        myView->Redraw();
    }
}
void OcctWidget::mousePressEvent(QMouseEvent *event)
{
    myLastMousePos = event->pos();
    int x = event->pos().x() * devicePixelRatio();
    int y = event->pos().y() * devicePixelRatio();

    if (event->button() == Qt::LeftButton) {
        myContext->MoveTo(x, y, myView, Standard_True);

        if (event->modifiers() & Qt::ShiftModifier) {
            myContext->SelectDetected(AIS_SelectionScheme_XOR);
        } else {
            myContext->SelectDetected(AIS_SelectionScheme_Replace);
        }

        myView->Redraw();
        myContext->InitSelected();
        if (myContext->HasSelectedShape()) {

            // ✅ THE SHIELD: If we clicked the ViewCube, STOP here and let it move the camera!
            Handle(AIS_InteractiveObject) selObj = myContext->SelectedInteractive();
            if (!selObj.IsNull() && selObj->DynamicType() == STANDARD_TYPE(AIS_ViewCube)) {
                myContext->ClearSelected(Standard_False);
                return;
            }

            if (myIsSettingOriginMode) {
                // ... (Keep all your existing origin setup logic below here) ...
                // (Keep your existing origin setup logic here...)
                TopoDS_Shape selectedShape = myContext->SelectedShape();
                Bnd_Box boundingBox;
                BRepBndLib::Add(selectedShape, boundingBox);
                Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
                boundingBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
                gp_Pnt newOriginSnap((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, (zMin + zMax) / 2.0);

                QString msg = QString("Do you want to set the new Robot Origin here?\n\nX: %1\nY: %2\nZ: %3")
                                  .arg(newOriginSnap.X(), 0, 'f', 2).arg(newOriginSnap.Y(), 0, 'f', 2).arg(newOriginSnap.Z(), 0, 'f', 2);

                if (QMessageBox::question(this, "Confirm New Origin", msg) == QMessageBox::Yes) {
                    myCustomOrigin = newOriginSnap;
                    gp_Ax2 newCoords(myCustomOrigin, gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
                    Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(newCoords);
                    myOriginMarker->SetComponent(placement);
                    myContext->Redisplay(myOriginMarker, Standard_True);
                    emit statusUpdate("🎯 New Local Origin Set Successfully!");
                } else {
                    emit statusUpdate("Origin Setup Cancelled.");
                }

                myIsSettingOriginMode = false;
                myContext->ClearSelected(Standard_True);
                return;
            }

            // ==========================================
            // NEW ROLE CHECKING LOGIC
            // ==========================================
            if (myRole == MainRole) {
                // Left Window: Isolate the part and send to Right Window
                TopoDS_Shape selectedShape = myContext->SelectedShape();
                emit partSelectedForIsolation(selectedShape);
                myContext->ClearSelected(Standard_True);
            }
            else if (myRole == SideRole) {
                // ✅ FIX: Don't extract immediately! Just notify the UI that a shape is selected.
                // The user must click "GET POINTS" to actually calculate the math.
                emit selectionChanged(myContext->HasSelectedShape());
            }
        }
    }
    // ==========================================
    // ✅ NEW FIX: Tell the camera to start rotating!
    // ==========================================
    else if (event->button() == Qt::RightButton) {
        myView->StartRotation(x, y);
    }
}


void OcctWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x() * devicePixelRatio();
    int y = event->pos().y() * devicePixelRatio();

    if (event->buttons() & Qt::RightButton) {
        myView->Rotation(x, y);
    } else if (event->buttons() & Qt::MiddleButton) {
        int lastX = myLastMousePos.x() * devicePixelRatio();
        int lastY = myLastMousePos.y() * devicePixelRatio();
        myView->Pan(x - lastX, lastY - y);
    } else {
        myContext->MoveTo(x, y, myView, Standard_True);
    }
    myLastMousePos = event->pos();
}

void OcctWidget::wheelEvent(QWheelEvent *event)
{
    myView->Zoom(0, 0, event->angleDelta().y() / 10, 0);
}


// ==========================================================
// ISOLATED PART DISPLAY (Right Window)
// ==========================================================
void OcctWidget::displayIsolatedPart(const TopoDS_Shape& shape)
{
    if (myView.IsNull()) initOCCT();

    myContext->RemoveAll(Standard_False);

    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    myContext->SetDisplayMode(aisShape, 1, Standard_False);

    // Change to Silver/Grey so the Cyan hover-blink is highly visible
    myContext->SetColor(aisShape, Quantity_NOC_GRAY75, Standard_False);
    myContext->SetMaterial(aisShape, Graphic3d_NOM_ALUMINIUM, Standard_False);

    myContext->Display(aisShape, Standard_False);

    // Force the shape to use the dropdown's rule, not the default!
    setSelectionMode(myCurrentSelectionMode);

    myView->FitAll();
    myView->Redraw();
}

// ==========================================================
// CALIBRATION OFFSET (Moves the Table/Square)
// ==========================================================
void OcctWidget::offsetWorkpiece(double dx, double dy, double dz)
{
    if (myLoadedPart.IsNull()) {
        emit statusUpdate("⚠️ Load a part first before moving it.");
        return;
    }

    // Create a 3D Translation Vector
    gp_Trsf transform;
    transform.SetTranslation(gp_Vec(dx, dy, dz));
    TopLoc_Location loc(transform);

    // Apply the offset to the part
    myContext->SetLocation(myLoadedPart, loc);
    myContext->UpdateCurrentViewer();

    emit statusUpdate(QString("📏 Part Calibrated to Robot Base -> X:%1, Y:%2, Z:%3").arg(dx).arg(dy).arg(dz));
}



// ==========================================
// 1. TRIGGER THE ASYNC LOADER
// ==========================================
void OcctWidget::loadDefaultRobot()
{
    if (myView.IsNull()) initOCCT();

    // Prevent crashing if the user spam-clicks the Load button
    if (myCurrentLoadIndex != -1) return;

    myCurrentLoadIndex = 0; // Start at link0

    emit statusUpdate("⏳ Loading STL Robot Base (link0)...");

    // Trigger the very first part to load after a tiny 50ms UI pause
    QTimer::singleShot(50, this, &OcctWidget::loadNextRobotLink);
}


void OcctWidget::loadNextRobotLink()
{
    if (myCurrentLoadIndex > 5) {
        myBaseTriad = createThickTriad(1.5);
        myTipTriad = createThickTriad(1.2);

        myContext->SetDisplayMode(myBaseTriad, 1, Standard_False);
        myContext->SetDisplayMode(myTipTriad, 1, Standard_False);

        gp_Trsf globalRot;
        globalRot.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,0,1)), -M_PI / 2.0);
        myContext->SetLocation(myBaseTriad, TopLoc_Location(globalRot));

        myContext->Display(myBaseTriad, Standard_False);
        myContext->Display(myTipTriad, Standard_False);

        myContext->Deactivate(myBaseTriad);
        myContext->Deactivate(myTipTriad);

        updateRobotPosture(0, 0, 0, 0, 0, 0);
        setSelectionMode(myCurrentSelectionMode);

        emit statusUpdate("✅ Successfully loaded 6 STL robot links.");

        myCurrentLoadIndex = -1;
        emit robotLoadComplete();

        // ==========================================================
        // ✅ THE FIX: DELAYED CAMERA CENTERING
        // We wait 250ms to guarantee the Qt layout has completely
        // finished stretching to fullscreen before we calculate the zoom.
        // ==========================================================
        QTimer::singleShot(250, this, [this]() {
            if (!myView.IsNull()) {
                myView->MustBeResized(); // Sync with X11 OS Window
                myView->FitAll();        // Perfectly frame the grid and robot
                myView->Redraw();        // Paint it to the screen
            }
        });

        return;
    }

    // 2. Setup the file path
    QString folderPath = "/home/texsonics/Documents/occtPro/step/";
    QString fileName = folderPath + QString("link%1.stl").arg(myCurrentLoadIndex);

    if (!QFile::exists(fileName)) {
        qDebug() << "❌ STL FILE NOT FOUND: Missing ->" << fileName;
        myCurrentLoadIndex++;
        QTimer::singleShot(50, this, &OcctWidget::loadNextRobotLink);
        return;
    }

    std::string stdFile = fileName.toStdString();

    // ✅ FIX: STL files are already meshed, they do not take a deflection argument!
    Handle(Poly_Triangulation) mesh = RWStl::ReadFile(stdFile.c_str());

    if (!mesh.IsNull()) {
        Handle(AIS_Triangulation) aisShape = new AIS_Triangulation(mesh);

        gp_Trsf zeroTrsf;
        myContext->SetLocation(aisShape, TopLoc_Location(zeroTrsf));

        Quantity_Color partColor;
        if (myCurrentLoadIndex == 0) partColor = Quantity_NOC_GRAY30;
        else if (myCurrentLoadIndex == 5) partColor = Quantity_NOC_GRAY75;
        else partColor = Quantity_Color(1.0, 0.4, 0.0, Quantity_TOC_RGB);

        myContext->SetColor(aisShape, partColor, Standard_False);
        myContext->Display(aisShape, Standard_False);
        myRobotLinks.push_back(aisShape);
        myContext->Deactivate(aisShape);
        myView->Redraw();
    }

    // 3. Increment the tracker to the next part
    myCurrentLoadIndex++;

    // 4. Update the UI text (Removed the broken updateRobotPosture call from here)
    emit statusUpdate(QString("⏳ Loading Raw Mesh (link%1.stl)...").arg(myCurrentLoadIndex - 1));

    QTimer::singleShot(50, this, &OcctWidget::loadNextRobotLink);
}







void OcctWidget::updateRobotPosture(double j1, double j2, double j3, double j4, double j5, double j6)
{
    if (myRobotLinks.empty()) return;

    // 1. Base Scale (Assuming STLs are in Meters, scale to MM)
    gp_Trsf baseTrsf;
    baseTrsf.SetScale(gp_Pnt(0,0,0), 1000.0);

    // ========================================================
    // ✅ NEW: GLOBAL WORLD ROTATION
    // This spins the ENTIRE assembled robot 90 degrees around Z
    // to match the facing direction of your main project!
    // ========================================================
    gp_Trsf globalRot;
    // NOTE: If the robot faces backward, just remove the negative sign -> (M_PI / 2.0)
    globalRot.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,0,1)), -M_PI / 2.0);

    // ========================================================
    // 2. TRUE ABSOLUTE KINEMATIC ORIGINS (In Millimeters)
    // ========================================================
    gp_Pnt orig1(0.0, 0.0, 0.0);       // J1 Pivot (Base to Turret)
    gp_Pnt orig2(150.0, 0.0, 462.0);   // J2 Pivot (Shoulder)
    gp_Pnt orig3(150.0, 0.0, 1062.0);  // J3 Pivot (Elbow)
    gp_Pnt orig4(150.0, 0.0, 1252.0);  // J4 Pivot (Forearm Roll)
    gp_Pnt orig5(837.0, 0.0, 1252.0);  // J5 Pivot (Wrist Pitch)
    gp_Pnt orig6(837.0, 0.0, 1252.0);  // J6 Pivot (Wrist Roll Flange)

    // ========================================================
    // 3. ROTATION AXES (Matching your KDL perfectly)
    // ========================================================
    gp_Dir axis1(0, 0, 1); // J1: RotZ
    gp_Dir axis2(0, 1, 0); // J2: RotY
    gp_Dir axis3(0, 1, 0); // J3: RotY
    gp_Dir axis4(1, 0, 0); // J4: RotX
    gp_Dir axis5(0, 1, 0); // J5: RotY
    gp_Dir axis6(1, 0, 0); // J6: RotX

    // 4. Create pure rotation transformations around those absolute pins
    gp_Trsf R1, R2, R3, R4, R5, R6;
    R1.SetRotation(gp_Ax1(orig1, axis1), j1);
    R2.SetRotation(gp_Ax1(orig2, axis2), j2);
    R3.SetRotation(gp_Ax1(orig3, axis3), j3);
    R4.SetRotation(gp_Ax1(orig4, axis4), j4);
    R5.SetRotation(gp_Ax1(orig5, axis5), j5);
    R6.SetRotation(gp_Ax1(orig6, axis6), j6);

    // ========================================================
    // 5. HIERARCHICAL ACCUMULATION
    // ========================================================
    gp_Trsf accum1 = R1;
    gp_Trsf accum2 = R1 * R2;
    gp_Trsf accum3 = R1 * R2 * R3;
    gp_Trsf accum4 = R1 * R2 * R3 * R4;
    gp_Trsf accum5 = R1 * R2 * R3 * R4 * R5;
    gp_Trsf accum6 = R1 * R2 * R3 * R4 * R5 * R6;

    // ========================================================
    // ✅ 6. APPLY TO STLs (Scale -> Internal Math -> Global Room Spin)
    // Order matters perfectly: globalRot * accum * baseTrsf
    // ========================================================
    if (myRobotLinks.size() > 0) myContext->SetLocation(myRobotLinks[0], TopLoc_Location(globalRot * baseTrsf));
    if (myRobotLinks.size() > 1) myContext->SetLocation(myRobotLinks[1], TopLoc_Location(globalRot * accum1 * baseTrsf));
    if (myRobotLinks.size() > 2) myContext->SetLocation(myRobotLinks[2], TopLoc_Location(globalRot * accum2 * baseTrsf));
    if (myRobotLinks.size() > 3) myContext->SetLocation(myRobotLinks[3], TopLoc_Location(globalRot * accum3 * baseTrsf));
    if (myRobotLinks.size() > 4) myContext->SetLocation(myRobotLinks[4], TopLoc_Location(globalRot * accum4 * baseTrsf));
    if (myRobotLinks.size() > 5) myContext->SetLocation(myRobotLinks[5], TopLoc_Location(globalRot * accum5 * baseTrsf));

    // If your STL robot has a 7th piece (the tool flange):
    if (myRobotLinks.size() > 6) myContext->SetLocation(myRobotLinks[6], TopLoc_Location(globalRot * accum6 * baseTrsf));

    // ========================================================
    // ✅ ATTACH THE ARROWS TO THE ROBOT
    // ========================================================
    // ... [Your existing R1 to accum6 math is above this] ...

    if (!myTipTriad.IsNull()) {
        gp_Trsf moveToTip;
        moveToTip.SetTranslation(gp_Vec(837.0 + 101.0, 0.0, 1252.0));

        // This is the absolute mathematical position of the Tool Center Point
        gp_Trsf finalTipTrsf = globalRot * accum6 * moveToTip;
        myContext->SetLocation(myTipTriad, TopLoc_Location(finalTipTrsf));

        // ========================================================
        // ✅ DRAW THE LIVE TRAJECTORY TRAIL
        // ========================================================
        // Get the absolute (X, Y, Z) point in the 3D room
        // ========================================================
        // ✅ OPTIMIZED: DRAW THE LIVE TRAJECTORY TRAIL
        // ========================================================
        gp_Pnt currentTCP = gp_Pnt(0,0,0).Transformed(finalTipTrsf);

        if (myTrajectoryPoints.empty() || myTrajectoryPoints.back().Distance(currentTCP) > 1.0) {
            myTrajectoryPoints.push_back(currentTCP);
            if (myTrajectoryPoints.size() > 500) {
                myTrajectoryPoints.erase(myTrajectoryPoints.begin());
            }

            // ✅ CRITICAL FIX: Only rebuild the CAD geometry every 100ms
            static qint64 lastTrailRedraw = 0;
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

            if (myTrajectoryPoints.size() > 1 && (currentTime - lastTrailRedraw > 100)) {
                lastTrailRedraw = currentTime;

                BRepBuilderAPI_MakePolygon polyMaker;
                for (const auto& pt : myTrajectoryPoints) {
                    polyMaker.Add(pt);
                }

                if (polyMaker.IsDone()) {
                    TopoDS_Wire wire = polyMaker.Wire();

                    if (myTrajectoryShape.IsNull()) {
                        myTrajectoryShape = new AIS_Shape(wire);
                        myContext->SetColor(myTrajectoryShape, Quantity_NOC_RED, Standard_False);//marker colour
                        myContext->SetWidth(myTrajectoryShape, 3.0, Standard_False);
                        myContext->Display(myTrajectoryShape, Standard_False);
                        myContext->Deactivate(myTrajectoryShape);
                    } else {
                        myTrajectoryShape->SetShape(wire);
                        myContext->Redisplay(myTrajectoryShape, Standard_False);
                    }
                }
            }
        }
    }

    // ========================================================
    // 🚀 THE FIX FOR THE HANGING TABS AND STUTTERING
    // ========================================================
    this->update();
}
void OcctWidget::drawRoomGrid()
{
    // Defined boundaries
    double minX = -2000.0, maxX = 2000.0;
    double minY = -2000.0, maxY = 2000.0;
    double minZ = 0.0, maxZ = 2000.0;
    double step = 100.0;

    TopoDS_Compound floorComp, backWallComp, leftWallComp;
    BRep_Builder builder;
    builder.MakeCompound(floorComp);
    builder.MakeCompound(backWallComp);
    builder.MakeCompound(leftWallComp);

    for (double x = minX; x <= maxX; x += step)
        builder.Add(floorComp, BRepBuilderAPI_MakeEdge(gp_Pnt(x, minY, minZ), gp_Pnt(x, maxY, minZ)));
    for (double y = minY; y <= maxY; y += step)
        builder.Add(floorComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, y, minZ), gp_Pnt(maxX, y, minZ)));

    for (double x = minX; x <= maxX; x += step)
        builder.Add(backWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(x, maxY, minZ), gp_Pnt(x, maxY, maxZ)));
    for (double z = minZ; z <= maxZ; z += step)
        builder.Add(backWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, maxY, z), gp_Pnt(maxX, maxY, z)));

    for (double y = minY; y <= maxY; y += step)
        builder.Add(leftWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, y, minZ), gp_Pnt(minX, y, maxZ)));
    for (double z = minZ; z <= maxZ; z += step)
        builder.Add(leftWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, minY, z), gp_Pnt(minX, maxY, z)));

    Handle(AIS_Shape) aisFloor = new AIS_Shape(floorComp);
    Handle(AIS_Shape) aisBack = new AIS_Shape(backWallComp);
    Handle(AIS_Shape) aisLeft = new AIS_Shape(leftWallComp);

    // ==========================================
    // ✅ LIGHT RED FLOOR (Perfect contrast for labels)
    // ==========================================
    Quantity_Color lightRedFloor(Quantity_NOC_INDIANRED1);
    myContext->SetColor(aisFloor, lightRedFloor, Standard_False);
    myContext->SetColor(aisBack, Quantity_NOC_STEELBLUE, Standard_False);
    myContext->SetColor(aisLeft, Quantity_NOC_SEAGREEN, Standard_False);

    myContext->Display(aisFloor, Standard_False);
    myContext->Display(aisBack, Standard_False);
    myContext->Display(aisLeft, Standard_False);
    myContext->Deactivate(aisFloor);
    myContext->Deactivate(aisBack);
    myContext->Deactivate(aisLeft);

    // ==========================================
    // ✅ TRUE 3D AXIS LABELS (Physical Size: 80mm)
    // ==========================================
    int textSize = 60;

    // RED LABELS (X-Axis)
    for (int x = -2000; x <= 2000; x += 100) {
        if (x == 0) continue;
        Handle(AIS_TextLabel) xLabel = new AIS_TextLabel();
        xLabel->SetText(TCollection_ExtendedString(x));
        xLabel->SetPosition(gp_Pnt(x, minY - 50.0, 0));
        xLabel->SetHeight(textSize);
        xLabel->SetColor(Quantity_NOC_RED);
        xLabel->SetZoomable(Standard_True); // True 3D object!
        myContext->Display(xLabel, Standard_False);
        myContext->Deactivate(xLabel);
    }

    // GREEN LABELS (Y-Axis)
    for (int y = -2000; y <= 2000; y += 100) {
        if (y == 0) continue;
        Handle(AIS_TextLabel) yLabel = new AIS_TextLabel();
        yLabel->SetText(TCollection_ExtendedString(y));
        yLabel->SetPosition(gp_Pnt(maxX + 50.0, y, 0));
        yLabel->SetHeight(textSize);
        yLabel->SetColor(Quantity_NOC_GREEN);
        yLabel->SetZoomable(Standard_True);
        myContext->Display(yLabel, Standard_False);
        myContext->Deactivate(yLabel);
    }

    // BLUE LABELS (Z-Axis)
    for (int z = 100; z <= 2000; z += 100) {
        Handle(AIS_TextLabel) zLabel = new AIS_TextLabel();
        zLabel->SetText(TCollection_ExtendedString(z));
        zLabel->SetPosition(gp_Pnt(maxX + 50.0, maxY, z));
        zLabel->SetHeight(textSize);
        zLabel->SetColor(Quantity_NOC_BLUE1);
        zLabel->SetZoomable(Standard_True);
        myContext->Display(zLabel, Standard_False);
        myContext->Deactivate(zLabel);
    }

    // ==========================================
    // ✅ PERFECT SMALL POSTER LOGO
    // ==========================================
    Handle(AIS_TextLabel) titleLabel = new AIS_TextLabel();
    titleLabel->SetText(TCollection_ExtendedString("TEXSONICS"));

    // Perfectly centered at X = 0.0
    titleLabel->SetPosition(gp_Pnt(0.0, maxY - 1.0, 1100));
    titleLabel->SetHeight(60); // <--- Shrunk down to a clean, professional poster size
    titleLabel->SetColor(Quantity_NOC_BLACK);
    titleLabel->SetZoomable(Standard_True);
    titleLabel->SetOrientation3D(gp_Ax2(gp_Pnt(0.0, maxY, 1200), gp_Dir(0, -1, 0), gp_Dir(1, 0, 0)));
    titleLabel->SetHJustification(Graphic3d_HTA_CENTER);
    myContext->Display(titleLabel, Standard_False);
    myContext->Deactivate(titleLabel);

    Handle(AIS_TextLabel) subLabel = new AIS_TextLabel();
    subLabel->SetText(TCollection_ExtendedString("R O B O T I C S"));

    subLabel->SetPosition(gp_Pnt(0.0, maxY - 1.0, 900));
    subLabel->SetHeight(30); // <--- Small subtitle size
    subLabel->SetColor(Quantity_NOC_BLACK);
    subLabel->SetZoomable(Standard_True);
    subLabel->SetOrientation3D(gp_Ax2(gp_Pnt(0.0, maxY, 900), gp_Dir(0, -1, 0), gp_Dir(1, 0, 0)));
    subLabel->SetHJustification(Graphic3d_HTA_CENTER);
    myContext->Display(subLabel, Standard_False);
    myContext->Deactivate(subLabel);



    myView->FitAll();
}

// ✅ NEW FUNCTION: Completely wipes the trail from memory and the 3D screen
void OcctWidget::clearMarks()
{
    // 1. If the shape exists on screen, remove it
    if (!myTrajectoryShape.IsNull()) {
        myContext->Remove(myTrajectoryShape, Standard_True); // Remove from viewer
        myTrajectoryShape.Nullify(); // Destroy the pointer
    }

    // 2. Clear all the saved X/Y/Z points so it doesn't instantly redraw
    myTrajectoryPoints.clear();

    emit statusUpdate("🧹 Trajectory marks cleared.");
}

// ==========================================================
// ✅ THE FIX: Wake up the Native X11 Window when the page flips
// ==========================================================
// ==========================================================
// ✅ THE FIX: Auto-trigger the layout sync (The "MAX" trick)
// ==========================================================
void OcctWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // Wait 100ms for Qt's layout and the Linux X11 Window Manager
    // to finish settling, then force the 3D canvas to snap to bounds!
    QTimer::singleShot(100, this, [this]() {
        if (!myView.IsNull()) {
            myView->MustBeResized();
            myView->Redraw();
            this->update(); // Force a Qt UI repaint just in case
        }
    });
}

void OcctWidget::clearLoadedPart()
{
    if (!myLoadedPart.IsNull()) {
        myContext->Remove(myLoadedPart, Standard_True);
        myLoadedPart.Nullify();
        emit statusUpdate("🗑️ STEP file cleared from view.");
        myView->Redraw();
    }
}

QString OcctWidget::getOriginText() const {
    return QString("X: %1 | Y: %2 | Z: %3")
    .arg(myCustomOrigin.X(), 0, 'f', 3)
        .arg(myCustomOrigin.Y(), 0, 'f', 3)
        .arg(myCustomOrigin.Z(), 0, 'f', 3);
}