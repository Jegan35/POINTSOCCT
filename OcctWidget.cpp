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
#include <STEPControl_Reader.hxx>
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


// ✅ ADD THE MISSING GRID HEADERS HERE:
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <cmath>


OcctWidget::OcctWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NativeWindow);
    setMouseTracking(true);

    // ====================================================
    // UPDATED: Set the default saving path for the CSV output
    // ====================================================
    myCSVPath = "/home/texsonics/Videos/extracted_paths.csv";
}

OcctWidget::~OcctWidget() {}

void OcctWidget::initOCCT()
{
    if (!myView.IsNull()) return;

    Handle(Aspect_DisplayConnection) displayConnection = new Aspect_DisplayConnection();
    myGraphicDriver = new OpenGl_GraphicDriver(displayConnection);

    myViewer = new V3d_Viewer(myGraphicDriver);
    myViewer->SetDefaultLights();
    myViewer->SetLightOn();

    //myViewer->SetRectangularGridValues(0, 0, 10, 10, 0);
    //myViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);

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

    myView->SetBgGradientColors(Quantity_NOC_BLACK, Quantity_NOC_GRAY30, Aspect_GFM_VER);
    myContext = new AIS_InteractiveContext(myViewer);

    // ✅ ADD THESE TWO LINES: Turn on 8x Hardware Anti-Aliasing!
    // This will make your custom grid lines perfectly smooth and straight.
    myView->ChangeRenderingParams().IsAntialiasingEnabled = Standard_True;
    myView->ChangeRenderingParams().NbMsaaSamples = 8;

    drawRoomGrid();
}

void OcctWidget::loadStepFile(const std::string& filePath)
{
    if (myView.IsNull()) initOCCT();

    // ❌ REMOVED: myContext->RemoveAll(Standard_True); <-- So the robot stays on screen!
    clearSelections();

    STEPControl_Reader reader;
    IFSelect_ReturnStatus stat = reader.ReadFile(filePath.c_str());

    if (stat == IFSelect_RetDone) {
        reader.TransferRoots();
        TopoDS_Shape shape = reader.OneShape();

        // Save it to our new memory variable
        myLoadedPart = new AIS_Shape(shape);
        myContext->SetDisplayMode(myLoadedPart, 1, Standard_False);

        // Give the workpiece a distinct color so it doesn't blend with the grey robot
        myContext->SetColor(myLoadedPart, Quantity_NOC_AZURE, Standard_False);
        myContext->Display(myLoadedPart, Standard_True);

        // ✅ CRITICAL FIX: Hard-lock the extraction origin to the Robot Base (0,0,0)
        myCustomOrigin = gp_Pnt(0.0, 0.0, 0.0);

        // ✅ NEW: Automatically push the part forward by 300mm on the X axis!
        // If your "forward" direction is actually the Y axis, change this to (0.0, 300.0, 0.0)
        offsetWorkpiece(300.0, 0.0, 0.0);

        myView->FitAll();
        myView->Redraw();
        setSelectionMode(myCurrentSelectionMode);

        emit statusUpdate("✅ Workpiece Loaded and Automatically pushed 300mm forward.");
    } else {
        emit statusUpdate("❌ Error: Failed to load STEP file.");
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

    // ✅ STORE THE MODE so we don't forget it when a new part loads
    myCurrentSelectionMode = mode;

    myContext->Deactivate();

    switch(mode) {
    case 1: myContext->Activate(AIS_Shape::SelectionMode(TopAbs_FACE)); break;
    case 2: myContext->Activate(AIS_Shape::SelectionMode(TopAbs_EDGE)); break;
    case 3: myContext->Activate(AIS_Shape::SelectionMode(TopAbs_WIRE)); break;
    default: myContext->Activate(0);
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

void OcctWidget::processCurrentSelection()
{
    if (myContext.IsNull() || !myContext->HasSelectedShape()) return;

    bool ok;
    double resolution = QInputDialog::getDouble(this, tr("Set Robot Path Resolution"), tr("Enter point spacing (mm):"), 2.0, 0.1, 100.0, 2, &ok);

    if (!ok) {
        emit statusUpdate("⚠️ Extraction cancelled by user.");
        myContext->ClearSelected(Standard_True);
        return;
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

        // ✅ FIX 3: If you clicked a FACE, tell OCCT to shade the whole face Red, not just the edges.
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

void OcctWidget::paintEvent(QPaintEvent *)
{
    if (myView.IsNull()) initOCCT();
    myView->Redraw();
}

void OcctWidget::resizeEvent(QResizeEvent *)
{
    if (!myView.IsNull()) myView->MustBeResized();
}

void OcctWidget::mousePressEvent(QMouseEvent *event)
{
    myLastMousePos = event->pos();

    if (event->button() == Qt::LeftButton) {
        int x = event->pos().x() * devicePixelRatio();
        int y = event->pos().y() * devicePixelRatio();

        myContext->MoveTo(x, y, myView, Standard_True);

        if (event->modifiers() & Qt::ShiftModifier) {
            myContext->SelectDetected(AIS_SelectionScheme_XOR);
        } else {
            myContext->SelectDetected(AIS_SelectionScheme_Replace);
        }

        myView->Redraw();
        myContext->InitSelected();

        if (myContext->HasSelectedShape()) {
            if (myIsSettingOriginMode) {
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
                // Right Window: Trigger YOUR exact extraction function!
                processCurrentSelection();
            }
        }
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

void OcctWidget::drawRoomGrid()
{
    // Defined boundaries
    double minX = -2000.0, maxX = 2000.0;
    double minY = -2000.0, maxY = 2000.0;
    double minZ = 0.0, maxZ = 3000.0;
    double step = 100.0;

    TopoDS_Compound floorComp, backWallComp, leftWallComp;
    BRep_Builder builder;
    builder.MakeCompound(floorComp);
    builder.MakeCompound(backWallComp);
    builder.MakeCompound(leftWallComp);

    // 1. FLOOR GRID (XY Plane)
    for (double x = minX; x <= maxX; x += step)
        builder.Add(floorComp, BRepBuilderAPI_MakeEdge(gp_Pnt(x, minY, minZ), gp_Pnt(x, maxY, minZ)));
    for (double y = minY; y <= maxY; y += step)
        builder.Add(floorComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, y, minZ), gp_Pnt(maxX, y, minZ)));

    // 2. BACK WALL GRID (XZ Plane)
    for (double x = minX; x <= maxX; x += step)
        builder.Add(backWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(x, maxY, minZ), gp_Pnt(x, maxY, maxZ)));
    for (double z = minZ; z <= maxZ; z += step)
        builder.Add(backWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, maxY, z), gp_Pnt(maxX, maxY, z)));

    // 3. LEFT WALL GRID (YZ Plane)
    for (double y = minY; y <= maxY; y += step)
        builder.Add(leftWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, y, minZ), gp_Pnt(minX, y, maxZ)));
    for (double z = minZ; z <= maxZ; z += step)
        builder.Add(leftWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, minY, z), gp_Pnt(minX, maxY, z)));

    // Convert into 3 separate display shapes
    Handle(AIS_Shape) aisFloor = new AIS_Shape(floorComp);
    Handle(AIS_Shape) aisBack = new AIS_Shape(backWallComp);
    Handle(AIS_Shape) aisLeft = new AIS_Shape(leftWallComp);

    // ✅ APPLY THE 3 DIFFERENT COLORS
    myContext->SetColor(aisFloor, Quantity_NOC_GRAY50, Standard_False);     // Grey
    myContext->SetColor(aisBack, Quantity_NOC_STEELBLUE, Standard_False);   // Blue
    myContext->SetColor(aisLeft, Quantity_NOC_SEAGREEN, Standard_False);    // Green

    // Display and make them un-clickable
    myContext->Display(aisFloor, Standard_False);
    myContext->Display(aisBack, Standard_False);
    myContext->Display(aisLeft, Standard_False);
    myContext->Deactivate(aisFloor);
    myContext->Deactivate(aisBack);
    myContext->Deactivate(aisLeft);

    // Center Red Dot
    Handle(AIS_Shape) centerDot = new AIS_Shape(BRepPrimAPI_MakeSphere(gp_Pnt(0,0,0), 15.0).Shape());
    myContext->SetColor(centerDot, Quantity_NOC_RED, Standard_False);
    myContext->Display(centerDot, Standard_False);
    myContext->Deactivate(centerDot);

    myView->FitAll();
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

// ==========================================
// 🚀 LIGHTNING-FAST RAW MESH STL LOADER
// ==========================================
void OcctWidget::loadNextRobotLink()
{
    // 1. Check if we are finished loading all 6 parts
    if (myCurrentLoadIndex > 5) {
        myView->FitAll();
        myContext->UpdateCurrentViewer();

        // Apply the selection mode (This will now ONLY apply to the STEP workpiece!)
        setSelectionMode(myCurrentSelectionMode);

        emit statusUpdate("✅ Successfully loaded 6 STL robot links instantly. Robot is visual only.");
        myCurrentLoadIndex = -1; // Reset tracker
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

    // ==========================================
    // 🚀 THE FIX: READ RAW MESH (NO MATH CONVERSION)
    // ==========================================
    Handle(Poly_Triangulation) mesh = RWStl::ReadFile(stdFile.c_str());

    if (!mesh.IsNull()) {
        // Wrap the raw mesh into a displayable object
        Handle(AIS_Triangulation) aisShape = new AIS_Triangulation(mesh);

        // Scale the STL
        gp_Trsf scaleTrsf;
        scaleTrsf.SetScale(gp_Pnt(0,0,0), 1000.0);

        // ✅ ROTATE: Turn the robot 90 degrees to face straight toward the camera
        // Note: If it faces the wrong way, just change the negative sign! (Standard_PI / 2.0)
        // ✅ ROTATE: Turn the robot 90 degrees using C++ M_PI
        gp_Trsf rotTrsf;
        rotTrsf.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,0,1)), -M_PI / 2.0);

        // Combine Rotation and Scale, then apply
        gp_Trsf finalTrsf = rotTrsf * scaleTrsf;
        TopLoc_Location loc(finalTrsf);
        myContext->SetLocation(aisShape, loc);

        // Apply Industrial Colors
        Quantity_Color partColor;
        if (myCurrentLoadIndex == 0) partColor = Quantity_NOC_GRAY30;
        else if (myCurrentLoadIndex == 5) partColor = Quantity_NOC_GRAY75;
        else partColor = Quantity_Color(1.0, 0.4, 0.0, Quantity_TOC_RGB);

        myContext->SetColor(aisShape, partColor, Standard_False);

        myContext->Display(aisShape, Standard_False);

        // ✅ THE GHOST TRICK: DEACTIVATE SELECTION
        myContext->Deactivate(aisShape);

        myView->Redraw();
    } else {
        qDebug() << "❌ Failed to read raw STL mesh:" << fileName;
    }

    // 3. Increment the tracker to the next part
    myCurrentLoadIndex++;

    // 4. Update the UI text and trigger the next loop
    if (myCurrentLoadIndex <= 5) {
        emit statusUpdate(QString("⏳ Loading Raw Mesh (link%1.stl)...").arg(myCurrentLoadIndex));
    }

    QTimer::singleShot(50, this, &OcctWidget::loadNextRobotLink);
}