/*
  Stub implementation of MidiTemplate for unit testing.
  Avoids pulling in QLCFile and other engine dependencies.
*/

#include "miditemplate.h"

MidiTemplate::MidiTemplate()
{
}

MidiTemplate::MidiTemplate(const MidiTemplate& templ)
    : m_description(templ.m_description)
    , m_initMessage(templ.m_initMessage)
{
}

MidiTemplate::~MidiTemplate()
{
}

MidiTemplate& MidiTemplate::operator=(const MidiTemplate& templ)
{
    if (this != &templ)
    {
        m_description = templ.m_description;
        m_initMessage = templ.m_initMessage;
    }
    return *this;
}

void MidiTemplate::setName(const QString& name)
{
    m_description = name;
}

QString MidiTemplate::name() const
{
    return m_description;
}

void MidiTemplate::setInitMessage(const QByteArray& message)
{
    m_initMessage = message;
}

QByteArray MidiTemplate::initMessage() const
{
    return m_initMessage;
}

MidiTemplate* MidiTemplate::loader(const QString& path)
{
    Q_UNUSED(path)
    return nullptr;
}

bool MidiTemplate::loadXML(QXmlStreamReader& doc)
{
    Q_UNUSED(doc)
    return false;
}
