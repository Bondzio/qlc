#ifndef TEST_H
#define TEST_H

#include <QScriptValue>
#include <QString>

class QScriptEngine;
class QSize;
class QDir;

typedef QVector<QVector<uint> > RGBMap;

class RGBScript
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    RGBScript();
    RGBScript(const RGBScript& s);
    ~RGBScript();

    /** Comparison operator. Uses simply fileName() == s.fileName(). */
    bool operator==(const RGBScript& s) const;

    /************************************************************************
     * Load & Evaluation
     ************************************************************************/
public:
    /** Load script contents from $file located in $dir */
    bool load(const QDir& dir, const QString& fileName);

    /** Get the filename for this script */
    QString fileName() const;

    /** Evaluate the script's contents and see if it checks out */
    bool evaluate();

private:
    QScriptEngine* m_engine;        //! The engine that runs the script
    QString m_fileName;             //! The file name that contains this script
    QString m_contents;             //! The file's contents

    /************************************************************************
     * Script API
     ************************************************************************/
public:
    /** Maximum step count for rgbMap() function */
    int rgbMapStepCount(const QSize& size);

    /** Get the RGBMap for the given step */
    RGBMap rgbMap(const QSize& size, uint rgb, int step);

    /** Get the name of the script */
    QString name();

    /** Get the name of the script's author */
    QString author();

    /** Get the script's API version. 0 == invalid or unevaluated script. */
    int apiVersion() const;

private:
    int m_apiVersion;               //! The API version that the script uses
    QScriptValue m_script;          //! The script itself
    QScriptValue m_rgbMap;          //! rgbMap() function
    QScriptValue m_rgbMapStepCount; //! rgbMapStepCount() function

    /************************************************************************
     * System & User Scripts
     ************************************************************************/
public:
    /**
     * Get all available (user & system) RGB Scripts. The returned scripts
     * have not been evaluated for validity.
     */
    static QList <RGBScript> scripts();

    /**
     * Get available scripts from the given directory path. The returned
     * scripts have not been evaluated for validity.
     */
    static QList <RGBScript> scripts(const QDir& path);

    /** The system RGBScript directory */
    static QDir systemScriptDirectory();

    /** The user RGBScript directory */
    static QDir userScriptDirectory();
};

#endif
