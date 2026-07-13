#include "E808Processor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new E808Processor();
}
