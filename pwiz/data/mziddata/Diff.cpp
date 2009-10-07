//
// $Id$
//
//
// Original author: Robert Burke <robetr.burke@proteowizard.org>
//
// Copyright 2009 Spielberg Family Center for Applied Proteomics
//   University of Southern California, Los Angeles, California  90033
//
// Licensed under the Apache License, Version 2.0 (the "License"); 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software 
// distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and 
// limitations under the License.
//

#define PWIZ_SOURCE

#include "Diff.hpp"
#include "boost/lexical_cast.hpp"
#include <string>
#include <cmath>
#include <stdexcept>

namespace pwiz {
namespace mziddata {
namespace diff_impl {

using namespace std;
using namespace boost;
using namespace boost::logic;

PWIZ_API_DECL
void diff(const string& a, 
          const string& b, 
          string& a_b, 
          string& b_a,
          const DiffConfig& config)
{
    a_b.clear();
    b_a.clear();
    
    if (a != b)
    {
        a_b = a;
        b_a = b;
    }
}

PWIZ_API_DECL
void diff(const tribool& a, 
          const tribool& b, 
          tribool& a_b, 
          tribool& b_a,
          const DiffConfig& config)
{
    a_b = indeterminate;
    b_a = indeterminate;
    
    if (a != b)
    {
        a_b = a;
        b_a = b;
    }
}

template <typename T>
void diff_numeric(const T& a, 
                  const T& b, 
                  T& a_b, 
                  T& b_a,
                  const DiffConfig& config)
{
    a_b = 0;
    b_a = 0;
    
    if (a != b)
    {
        a_b = a;
        b_a = b;
    }
}


template <>
void diff_numeric(const double& a,
                  const double& b,
                  double& a_b,
                  double& b_a,
                  const DiffConfig& config)
{
    a_b = 0;
    b_a = 0;

    if (fabs(a - b) > config.precision + std::numeric_limits<double>::epsilon())
    {
        a_b = fabs(a - b);
        b_a = fabs(a - b);
    }
}


PWIZ_API_DECL
void diff(const CV& a, 
          const CV& b, 
          CV& a_b, 
          CV& b_a,
          const DiffConfig& config)
{
    diff(a.URI, b.URI, a_b.URI, b_a.URI, config);
    diff(a.id, b.id, a_b.id, b_a.id, config);
    diff(a.fullName, b.fullName, a_b.fullName, b_a.fullName, config);
    diff(a.version, b.version, a_b.version, b_a.version, config);
}


PWIZ_API_DECL
void diff(CVID a,
          CVID b,
          CVID& a_b,
          CVID& b_a,
          const DiffConfig& config)
{
    a_b = b_a = CVID_Unknown;
    if (a!=b)  
    {
        a_b = a;
        b_a = b;
    }
}


PWIZ_API_DECL
void diff(const CVParam& a, 
          const CVParam& b, 
          CVParam& a_b, 
          CVParam& b_a,
          const DiffConfig& config)
{
    diff(a.cvid, b.cvid, a_b.cvid, b_a.cvid, config);

    // use precision to compare floating point values
    try
    {
        lexical_cast<int>(a.value);
        lexical_cast<int>(b.value);
    }
    catch (boost::bad_lexical_cast&)
    {
        try
        {
            double aValue = lexical_cast<double>(a.value);
            double bValue = lexical_cast<double>(b.value);
            double a_bValue, b_aValue;
            diff_numeric<double>(aValue, bValue, a_bValue, b_aValue, config);
            a_b.value = lexical_cast<string>(a_bValue);
            b_a.value = lexical_cast<string>(b_aValue);
        }
        catch (boost::bad_lexical_cast&)
        {
            diff(a.value, b.value, a_b.value, b_a.value, config);
        }
    }

    diff(a.units, b.units, a_b.units, b_a.units, config);

    // provide names for context
    if (!a_b.empty() && a_b.cvid==CVID_Unknown) a_b.cvid = a.cvid; 
    if (!b_a.empty() && b_a.cvid==CVID_Unknown) b_a.cvid = b.cvid; 
}


PWIZ_API_DECL
void diff(const UserParam& a, 
          const UserParam& b, 
          UserParam& a_b, 
          UserParam& b_a,
          const DiffConfig& config)
{
    diff(a.name, b.name, a_b.name, b_a.name, config);
    diff(a.value, b.value, a_b.value, b_a.value, config);
    diff(a.type, b.type, a_b.type, b_a.type, config);
    diff(a.units, b.units, a_b.units, b_a.units, config);

    // provide names for context
    if (!a_b.empty() && a_b.name.empty()) a_b.name = a.name; 
    if (!b_a.empty() && b_a.name.empty()) b_a.name = b.name; 
}


template <typename object_type>
void vector_diff(const vector<object_type>& a,
                 const vector<object_type>& b,
                 vector<object_type>& a_b,
                 vector<object_type>& b_a)
{
    // calculate set differences of two vectors

    a_b.clear();
    b_a.clear();

    for (typename vector<object_type>::const_iterator it=a.begin(); it!=a.end(); ++it)
        if (find(b.begin(), b.end(), *it) == b.end())
            a_b.push_back(*it);

    for (typename vector<object_type>::const_iterator it=b.begin(); it!=b.end(); ++it)
        if (find(a.begin(), a.end(), *it) == a.end())
            b_a.push_back(*it);
}


template <typename object_type>
struct HasID
{
    const string& id_;
    HasID(const string& id) : id_(id) {}
    bool operator()(const shared_ptr<object_type>& objectPtr) {return objectPtr->id == id_;}
};


template <typename object_type>
class Same
{
    public:

    Same(const object_type& object,
         const DiffConfig& config)
    :   mine_(object), config_(config)
    {}

    bool operator()(const object_type& yours)
    {
        // true iff yours is the same as mine
        return !Diff<object_type>(mine_, yours, config_);
    }

    private:
    const object_type& mine_;
    const DiffConfig& config_;
};


template <typename object_type>
void vector_diff_diff(const vector<object_type>& a,
                      const vector<object_type>& b,
                      vector<object_type>& a_b,
                      vector<object_type>& b_a,
                      const DiffConfig& config)
{
    // calculate set differences of two vectors, using diff on each object

    a_b.clear();
    b_a.clear();

    for (typename vector<object_type>::const_iterator it=a.begin(); it!=a.end(); ++it)
        if (find_if(b.begin(), b.end(), Same<object_type>(*it, config)) == b.end())
            a_b.push_back(*it);

    for (typename vector<object_type>::const_iterator it=b.begin(); it!=b.end(); ++it)
        if (find_if(a.begin(), a.end(), Same<object_type>(*it, config)) == a.end())
            b_a.push_back(*it);
}


template <typename object_type>
class SameDeep
{
    public:

    SameDeep(const object_type& object,
             const DiffConfig& config)
    :   mine_(object), config_(config)
    {}

    bool operator()(const shared_ptr<object_type>& yours)
    {
        // true iff yours is the same as mine
        return !Diff<object_type>(mine_, *yours, config_);
    }

    private:
    const object_type& mine_;
    const DiffConfig& config_;
};


template <typename object_type>
void vector_diff_deep(const vector< shared_ptr<object_type> >& a,
                      const vector< shared_ptr<object_type> >& b,
                      vector< shared_ptr<object_type> >& a_b,
                      vector< shared_ptr<object_type> >& b_a,
                      const DiffConfig& config)
{
    // calculate set differences of two vectors of ObjectPtrs (deep compare using diff)

    a_b.clear();
    b_a.clear();

    for (typename vector< shared_ptr<object_type> >::const_iterator it=a.begin(); it!=a.end(); ++it)
        if (find_if(b.begin(), b.end(), SameDeep<object_type>(**it, config)) == b.end())
            a_b.push_back(*it);

    for (typename vector< shared_ptr<object_type> >::const_iterator it=b.begin(); it!=b.end(); ++it)
        if (find_if(a.begin(), a.end(), SameDeep<object_type>(**it, config)) == a.end())
            b_a.push_back(*it);
}


template <typename object_type>
void ptr_diff(const shared_ptr<object_type>& a,
              const shared_ptr<object_type>& b,
              shared_ptr<object_type>& a_b,
              shared_ptr<object_type>& b_a,
              const DiffConfig& config)
{
    if (!a.get() && !b.get()) return;

    shared_ptr<object_type> a_temp = a.get() ? a : shared_ptr<object_type>(new object_type);
    shared_ptr<object_type> b_temp = b.get() ? b : shared_ptr<object_type>(new object_type);

    if (!a_b.get()) a_b = shared_ptr<object_type>(new object_type);
    if (!b_a.get()) b_a = shared_ptr<object_type>(new object_type);
    diff(*a_temp, *b_temp, *a_b, *b_a, config);

    if (a_b->empty()) a_b = shared_ptr<object_type>();
    if (b_a->empty()) b_a = shared_ptr<object_type>();
}

PWIZ_API_DECL
void diff(const ParamContainer& a, 
          const ParamContainer& b, 
          ParamContainer& a_b, 
          ParamContainer& b_a,
          const DiffConfig& config)
{
    vector_diff(a.cvParams, b.cvParams, a_b.cvParams, b_a.cvParams);
    vector_diff(a.userParams, b.userParams, a_b.userParams, b_a.userParams);
}


// measure maximum relative difference between elements in the vectors
double maxdiff(const vector<double>& a, const vector<double>& b)
{
    if (a.size() != b.size()) 
        throw runtime_error("[Diff::maxdiff()] Sizes differ.");

    vector<double>::const_iterator i = a.begin(); 
    vector<double>::const_iterator j = b.begin(); 

    double max = 0;

    for (; i!=a.end(); ++i, ++j)
    {
        double denominator = min(*i, *j);
        if (denominator == 0) denominator = 1;
        double current = fabs(*i - *j)/denominator;
        if (max < current) max = current;

    }

    return max;
}


const char* userParamName_FragmentArrayDifference_ = "FragmentArray difference";

PWIZ_API_DECL
void diff(const FragmentArray& a,
          const FragmentArray& b,
          FragmentArray& a_b,
          FragmentArray& b_a,
          const DiffConfig& config)
{
    if (a.values.size() != b.values.size())
    {
        a_b.params.userParams.push_back(UserParam("Binary data array size: " + 
                                           lexical_cast<string>(a.values.size())));
        b_a.params.userParams.push_back(UserParam("Binary data array size: " + 
                                           lexical_cast<string>(b.values.size())));
    }
    else
    {
        double max = maxdiff(a.values, b.values);
       
        if (max > config.precision + numeric_limits<double>::epsilon())
        {
            a_b.params.userParams.push_back(UserParam(userParamName_FragmentArrayDifference_,
                                               lexical_cast<string>(max),
                                               "xsd:float"));
            b_a.params.userParams.push_back(UserParam(userParamName_FragmentArrayDifference_,
                                               lexical_cast<string>(max),
					       "xsd:float"));
        }
    }
    ptr_diff(a.measurePtr, b.measurePtr, a_b.measurePtr, b_a.measurePtr, config);
}


PWIZ_API_DECL
void diff(const Measure& a,
          const Measure& b,
          Measure& a_b,
          Measure& b_a,
          const DiffConfig& config)
{
    diff((IdentifiableType&)a, (IdentifiableType&)b,
         (IdentifiableType&)a_b, (IdentifiableType&)b_a, config);
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup, config);
}


PWIZ_API_DECL
void diff(const ModParam& a,
          const ModParam& b,
          ModParam& a_b,
          ModParam& b_a,
          const DiffConfig& config)
{
    diff_numeric(a.massDelta, b.massDelta, a_b.massDelta, b_a.massDelta, config);
    diff(a.residues, b.residues, a_b.residues, b_a.residues, config);
    diff(a.cvParams, b.cvParams, a_b.cvParams, b_a.cvParams, config);
}


PWIZ_API_DECL
void diff(const SearchModification& a,
          const SearchModification& b,
          SearchModification& a_b,
          SearchModification& b_a,
          const DiffConfig& config)
{
    if (a.fixedMod != b.fixedMod)
    {
        a_b.fixedMod = a.fixedMod;
        b_a.fixedMod = b.fixedMod;
    }
    diff(a.modParam, b.modParam, a_b.modParam, b_a.modParam, config);
    diff(a.specificityRules, b.specificityRules,
         a_b.specificityRules, b_a.specificityRules, config);
}


PWIZ_API_DECL
void diff(const IonType& a,
          const IonType& b,
          IonType& a_b,
          IonType& b_a,
          const DiffConfig& config)
{
    diff_numeric(a.charge, b.charge, a_b.charge, b_a.charge, config);
    vector_diff(a.index, b.index, a_b.index, b_a.index);
    vector_diff_deep(a.fragmentArray, b.fragmentArray,
         a_b.fragmentArray, b_a.fragmentArray, config);
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup,
    config);
}



PWIZ_API_DECL
void diff(const Material& a,
          const Material& b,
          Material& a_b,
          Material& b_a,
          const DiffConfig& config)
{
    diff(a.contactRole, b.contactRole, a_b.contactRole, b_a.contactRole, config);
    diff(a.cvParams, b.cvParams, a_b.cvParams, b_a.cvParams, config);
}

PWIZ_API_DECL
void diff(const DataCollection& a,
          const DataCollection& b,
          DataCollection& a_b,
          DataCollection& b_a,
          const DiffConfig& config)
{
    diff(a.inputs, b.inputs, a_b.inputs, b_a.inputs, config);
    diff(a.analysisData, b.analysisData, a_b.analysisData, b_a.analysisData, config);
}


PWIZ_API_DECL
void diff(const PeptideEvidence& a,
          const PeptideEvidence& b,
          PeptideEvidence& a_b,
          PeptideEvidence& b_a,
          const DiffConfig& config)
{
    diff((IdentifiableType&)a, (IdentifiableType&)b,
         (IdentifiableType&)a_b, (IdentifiableType&)b_a, config);
    ptr_diff(a.dbSequencePtr, b.dbSequencePtr, a_b.dbSequencePtr,
             b_a.dbSequencePtr, config);
    diff_numeric(a.start, b.start, a_b.start, b_a.start, config);
    diff_numeric(a.end, b.end, a_b.end, b_a.end, config);
    diff(a.pre, b.pre, a_b.pre, b_a.pre, config);
    diff(a.post, b.post, a_b.post, b_a.post, config);
    ptr_diff(a.translationTablePtr, b.translationTablePtr,
         a_b.translationTablePtr, b_a.translationTablePtr, config);
    diff_numeric(a.frame, b.frame, a_b.frame, b_a.frame, config);
    if(a.isDecoy != b.isDecoy)
    {
        a_b.isDecoy = a.isDecoy;
        b_a.isDecoy = b.isDecoy;
    }
    diff_numeric(a.missedCleavages, b.missedCleavages,
                 a_b.missedCleavages, b_a.missedCleavages, config);
    
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup, config);
}


PWIZ_API_DECL
void diff(const SpectrumIdentificationItem& a,
          const SpectrumIdentificationItem& b,
          SpectrumIdentificationItem& a_b,
          SpectrumIdentificationItem& b_a,
          const DiffConfig& config)
{
    diff_numeric(a.chargeState, b.chargeState, a_b.chargeState, b_a.chargeState,
                 config);

    diff_numeric(a.experimentalMassToCharge, b.experimentalMassToCharge,
                 a_b.experimentalMassToCharge, b_a.experimentalMassToCharge,
                 config);
    diff_numeric(a.calculatedMassToCharge, b.calculatedMassToCharge,
                 a_b.calculatedMassToCharge, b_a.calculatedMassToCharge,
                 config);
    diff_numeric(a.calculatedPI, b.calculatedPI,
                 a_b.calculatedPI, b_a.calculatedPI,
                 config);
    ptr_diff(a.peptidePtr, b.peptidePtr,
                 a_b.peptidePtr, b_a.peptidePtr,
                 config);
    diff_numeric(a.rank, b.rank, a_b.rank, b_a.rank, config);

    if(a.passThreshold != b.passThreshold)
    {
        a_b.passThreshold = a.passThreshold;
        b_a.passThreshold = b.passThreshold;
    }

    ptr_diff(a.massTablePtr, b.massTablePtr,
                 a_b.massTablePtr, b_a.massTablePtr, config);
    
    ptr_diff(a.samplePtr, b.samplePtr,
                 a_b.samplePtr, b_a.samplePtr, config);

    vector_diff_deep(a.peptideEvidence, b.peptideEvidence,
                     a_b.peptideEvidence, b_a.peptideEvidence, config);

    vector_diff_deep(a.fragmentation, b.fragmentation,
                     a_b.fragmentation, b_a.fragmentation, config);
    
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup,
    config);
}


PWIZ_API_DECL
void diff(const SpectrumIdentificationResult& a,
          const SpectrumIdentificationResult& b,
          SpectrumIdentificationResult& a_b,
          SpectrumIdentificationResult& b_a,
          const DiffConfig& config)
{
    diff(a.spectrumID, b.spectrumID, a_b.spectrumID, b_a.spectrumID, config);
    ptr_diff(a.spectraDataPtr, b.spectraDataPtr,
         a_b.spectraDataPtr, b_a.spectraDataPtr, config);

    vector_diff_deep(a.spectrumIdentificationItem, b.spectrumIdentificationItem,
         a_b.spectrumIdentificationItem, b_a.spectrumIdentificationItem,
         config);
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup,
         config);
}

PWIZ_API_DECL
void diff(const SpectrumIdentificationListPtr a,
          const SpectrumIdentificationListPtr b,
          SpectrumIdentificationListPtr a_b,
          SpectrumIdentificationListPtr b_a,
          const DiffConfig& config)
{
    ptr_diff(a, b, a_b, b_a, config);
}


PWIZ_API_DECL
void diff(const SpectrumIdentificationList& a,
          const SpectrumIdentificationList& b,
          SpectrumIdentificationList& a_b,
          SpectrumIdentificationList& b_a,
          const DiffConfig& config)
{
    diff_numeric(a.numSequencesSearched, b.numSequencesSearched,
                 a_b.numSequencesSearched, b_a.numSequencesSearched, config);
    vector_diff_deep(a.fragmentationTable, b.fragmentationTable,
                     a_b.fragmentationTable, b_a.fragmentationTable,
                     config);
    vector_diff_deep(a.spectrumIdentificationResult,
                     b.spectrumIdentificationResult,
                     a_b.spectrumIdentificationResult,
                     b_a.spectrumIdentificationResult, config);
}

PWIZ_API_DECL
void diff(const ProteinDetectionHypothesis& a,
          const ProteinDetectionHypothesis& b,
          ProteinDetectionHypothesis& a_b,
          ProteinDetectionHypothesis& b_a,
          const DiffConfig& config)
{
    ptr_diff(a.dbSequencePtr, b.dbSequencePtr,
         a_b.dbSequencePtr, b_a.dbSequencePtr, config);
    if (a.passThreshold != b.passThreshold)
    {
        a_b.passThreshold = a.passThreshold;
        b_a.passThreshold = b.passThreshold;
    }
    vector_diff(a.peptideHypothesis, b.peptideHypothesis,
                a_b.peptideHypothesis, b_a.peptideHypothesis);
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup, config);
}

PWIZ_API_DECL
void diff(const ProteinAmbiguityGroup& a,
          const ProteinAmbiguityGroup& b,
          ProteinAmbiguityGroup& a_b,
          ProteinAmbiguityGroup& b_a,
          const DiffConfig& config)
{
    vector_diff_deep(a.proteinDetectionHypothesis, b.proteinDetectionHypothesis, a_b.proteinDetectionHypothesis, b_a.proteinDetectionHypothesis, config);
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup, config);
}

PWIZ_API_DECL
void diff(const ProteinDetectionList& a,
          const ProteinDetectionList& b,
          ProteinDetectionList& a_b,
          ProteinDetectionList& b_a,
          const DiffConfig& config)
{    
    diff((const IdentifiableType&)a, (const IdentifiableType&)b,
         (IdentifiableType&)a_b, (IdentifiableType&)b_a, config);
    vector_diff_deep(a.proteinAmbiguityGroup, b.proteinAmbiguityGroup,
                     a_b.proteinAmbiguityGroup, b_a.proteinAmbiguityGroup,
                     config);
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup, config);
}

PWIZ_API_DECL
void diff(const AnalysisData& a,
          const AnalysisData& b,
          AnalysisData& a_b,
          AnalysisData& b_a,
          const DiffConfig& config)
{
    vector_diff_deep(a.spectrumIdentificationList,
                     b.spectrumIdentificationList,
                     a_b.spectrumIdentificationList,
                     b_a.spectrumIdentificationList, config);
    ptr_diff(a.proteinDetectionListPtr, b.proteinDetectionListPtr,
                     a_b.proteinDetectionListPtr, b_a.proteinDetectionListPtr,
                     config);
}

PWIZ_API_DECL
void diff(const SearchDatabase& a,
          const SearchDatabase& b,
          SearchDatabase& a_b,
          SearchDatabase& b_a,
          const DiffConfig& config)
{
    diff(a.version, b.version, a_b.version, b_a.version, config);
    diff(a.releaseDate, b.releaseDate, a_b.releaseDate, b_a.releaseDate, config);
    if (a.numDatabaseSequences != b.numDatabaseSequences)
    {
        a_b.numDatabaseSequences = a.numDatabaseSequences;
        b_a.numDatabaseSequences = b.numDatabaseSequences;
    }

    if (a.numResidues != b.numResidues)
    {
        a_b.numResidues = a.numResidues;
        b_a.numResidues = b.numResidues;
    }

    diff(a.fileFormat, b.fileFormat, a_b.fileFormat, b_a.fileFormat, config);
    diff(a.DatabaseName, b.DatabaseName, a_b.DatabaseName, b_a.DatabaseName, config);
}

PWIZ_API_DECL
void diff(const SourceFile& a,
          const SourceFile& b,
          SourceFile& a_b,
          SourceFile& b_a,
          const DiffConfig& config)
{
    diff(a.location, b.location, a_b.location, b_a.location, config);
    diff(a.fileFormat, b.fileFormat, a_b.fileFormat, b_a.fileFormat, config);
    vector_diff_diff(a.externalFormatDocumentation,
                     b.externalFormatDocumentation,
                     a_b.externalFormatDocumentation,
                     b_a.externalFormatDocumentation, config);
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup, config);
}

PWIZ_API_DECL
void diff(const SpectraData& a,
          const SpectraData& b,
          SpectraData& a_b,
          SpectraData& b_a,
          const DiffConfig& config)
{
    diff(a.location, b.location, a_b.location, b_a.location, config);
    vector_diff_diff(a.externalFormatDocumentation,
                     b.externalFormatDocumentation,
                     a_b.externalFormatDocumentation,
                     b_a.externalFormatDocumentation, config);
    diff(a.fileFormat, b.fileFormat, a_b.fileFormat, b_a.fileFormat, config);
}

PWIZ_API_DECL
void diff(const Inputs& a,
          const Inputs& b,
          Inputs& a_b,
          Inputs& b_a,
          const DiffConfig& config)
{
    vector_diff_deep(a.sourceFile, b.sourceFile,
                     a_b.sourceFile, b_a.sourceFile, config);
    vector_diff_deep(a.searchDatabase, b.searchDatabase,
                     a_b.searchDatabase, b_a.searchDatabase, config);
    vector_diff_deep(a.spectraData, b.spectraData,
                     a_b.spectraData, b_a.spectraData, config);
}

PWIZ_API_DECL
void diff(const Enzyme& a,
          const Enzyme& b,
          Enzyme& a_b,
          Enzyme& b_a,
          const DiffConfig& config)
{
    diff(a.id, b.id, a_b.id, b_a.id,config);
    diff(a.nTermGain, b.nTermGain, a_b.nTermGain, b_a.nTermGain,config);
    diff(a.cTermGain, b.cTermGain, a_b.cTermGain, b_a.cTermGain,config);
    diff(a.semiSpecific, b.semiSpecific, a_b.semiSpecific, b_a.semiSpecific,config);
    diff_numeric(a.missedCleavages, b.missedCleavages, a_b.missedCleavages, b_a.missedCleavages,config);
    diff_numeric(a.minDistance, b.minDistance, a_b.minDistance, b_a.minDistance,config);
    diff(a.siteRegexp, b.siteRegexp, a_b.siteRegexp, b_a.siteRegexp,config);
    diff(a.enzymeName, b.enzymeName, a_b.enzymeName, b_a.enzymeName,config);
}

PWIZ_API_DECL
void diff(const Enzymes& a,
          const Enzymes& b,
          Enzymes& a_b,
          Enzymes& b_a,
          const DiffConfig& config)
{
    diff(a.independent, b.independent, a_b.independent, b_a.independent,config);
    vector_diff_deep(a.enzymes, b.enzymes, a_b.enzymes, b_a.enzymes,config);
}

PWIZ_API_DECL
void diff(const MassTable& a,
          const MassTable& b,
          MassTable& a_b,
          MassTable& b_a,
          const DiffConfig& config)
{
    diff(a.id, b.id, a_b.id, b_a.id, config);
    diff(a.msLevel, b.msLevel, a_b.msLevel, b_a.msLevel, config);
    vector_diff_deep(a.residues, b.residues, a_b.residues, b_a.residues, config);
    vector_diff_deep(a.ambiguousResidue, b.ambiguousResidue, a_b.ambiguousResidue, b_a.ambiguousResidue, config); 
}


PWIZ_API_DECL
void diff(const Residue& a,
          const Residue& b,
          Residue& a_b,
          Residue& b_a,
          const DiffConfig& config)
{
    diff(a.Code, b.Code, a_b.Code, b_a.Code, config);
    diff_numeric(a.Mass, b.Mass, a_b.Mass, b_a.Mass, config);
}

PWIZ_API_DECL
void diff(const AmbiguousResidue& a,
          const AmbiguousResidue& b,
          AmbiguousResidue& a_b,
          AmbiguousResidue& b_a,
          const DiffConfig& config)
{
    diff(a.Code, b.Code, a_b.Code, b_a.Code, config);
    diff(a.params, b.params, a_b.params, b_a.params, config);
}

PWIZ_API_DECL
void diff(const Filter& a,
          const Filter& b,
          Filter& a_b,
          Filter& b_a,
          const DiffConfig& config)
{
    diff(a.filterType, b.filterType, a_b.filterType, b_a.filterType, config);
    diff(a.include, b.include, a_b.include, b_a.include, config);
    diff(a.exclude, b.exclude, a_b.exclude, b_a.exclude, config);
}

PWIZ_API_DECL
void diff(const SpectrumIdentificationProtocol& a,
          const SpectrumIdentificationProtocol& b,
          SpectrumIdentificationProtocol& a_b,
          SpectrumIdentificationProtocol& b_a,
          const DiffConfig& config)
{
    ptr_diff(a.analysisSoftwarePtr, b.analysisSoftwarePtr,
         a_b.analysisSoftwarePtr, b_a.analysisSoftwarePtr,
         config);
    diff(a.searchType, b.searchType, a_b.searchType, b_a.searchType, config);
    diff(a.additionalSearchParams, b.additionalSearchParams,
         a_b.additionalSearchParams, b_a.additionalSearchParams, config);
    diff(a.searchType, b.searchType,
                     a_b.searchType, b_a.searchType, config);
    diff(a.enzymes, b.enzymes, a_b.enzymes, b_a.enzymes, config);
    diff(a.massTable, b.massTable, a_b.massTable, b_a.massTable, config);
    diff(a.fragmentTolerance, b.fragmentTolerance, a_b.fragmentTolerance, b_a.fragmentTolerance, config);
    diff(a.parentTolerance, b.parentTolerance, a_b.parentTolerance, b_a.parentTolerance, config);
    diff(a.threshold, b.threshold, a_b.threshold, b_a.threshold, config);
    vector_diff_deep(a.databaseFilters, b.databaseFilters, a_b.databaseFilters, b_a.databaseFilters, config);
}

PWIZ_API_DECL
void diff(const ProteinDetectionProtocol& a,
          const ProteinDetectionProtocol& b,
          ProteinDetectionProtocol& a_b,
          ProteinDetectionProtocol& b_a,
          const DiffConfig& config)
{
    ptr_diff(a.analysisSoftwarePtr, b.analysisSoftwarePtr, a_b.analysisSoftwarePtr, b_a.analysisSoftwarePtr, config);
    diff(a.analysisParams, b.analysisParams, a_b.analysisParams, b_a.analysisParams, config);
    diff(a.threshold, b.threshold, a_b.threshold, b_a.threshold, config);
}

PWIZ_API_DECL
void diff(const AnalysisProtocolCollection& a,
          const AnalysisProtocolCollection& b,
          AnalysisProtocolCollection& a_b,
          AnalysisProtocolCollection& b_a,
          const DiffConfig& config)
{
    vector_diff_deep(a.spectrumIdentificationProtocol,
                     b.spectrumIdentificationProtocol,
                     a_b.spectrumIdentificationProtocol,
                     b_a.spectrumIdentificationProtocol,
                     config);
    vector_diff_deep(a.proteinDetectionProtocol,
                     b.proteinDetectionProtocol,
                     a_b.proteinDetectionProtocol,
                     b_a.proteinDetectionProtocol,
                     config);
}

PWIZ_API_DECL
void diff(const Contact& a,
          const Contact& b,
          Contact& a_b,
          Contact& b_a,
          const DiffConfig& config)
{
    diff((IdentifiableType&)a, (IdentifiableType&)b,
         (IdentifiableType&)a_b, (IdentifiableType&)b_a, config);
    diff(a.address, b.address, a_b.address, b_a.address, config);
    diff(a.phone, b.phone, a_b.phone, b_a.phone, config);
    diff(a.email, b.email, a_b.email, b_a.email, config);
    diff(a.fax, b.fax, a_b.fax, b_a.fax, config);
    diff(a.tollFreePhone, b.tollFreePhone, a_b.tollFreePhone,
         b_a.tollFreePhone, config);
}


const char* ContactPtr_diff_string_ = "Contact type different ";

PWIZ_API_DECL
void diff(const ContactPtr a,
          const ContactPtr b,
          ContactPtr& a_b,
          ContactPtr& b_a,
          const DiffConfig& config)
{
    Person* a_person = dynamic_cast<Person*>(a.get());
    Person* b_person = dynamic_cast<Person*>(b.get());
    
    Organization* a_organization = dynamic_cast<Organization*>(a.get());
    Organization* b_organization = dynamic_cast<Organization*>(b.get());
    
    if (a_person && b_person)
    {
        a_b = ContactPtr(new Person());
        b_a = ContactPtr(new Person());
        diff(*a_person, *b_person,
                 (Person&)*a_b, (Person&)*b_a, config);
    }
    else if (a_organization && b_organization)
    {
        a_b = ContactPtr(new Organization());
        b_a = ContactPtr(new Organization());
        diff(*a_organization, *b_organization,
                 (Organization&)*a_b, (Organization&)*b_a, config);
    }
    else
    {
        // TODO add a UserParam to indicate that we're using the
        // generic diff
        
        //a_b = ContactPtr(new Contact());
        //b_a = ContactPtr(new Contact());
        ptr_diff(a, b, a_b, b_a, config);

        string a_type = (a_person ? "Person" : (a_organization ? "Organization" : "Contact"));
        string b_type = (b_person ? "Person" : (b_organization ? "Organization" : "Contact"));
        //a_b->params.userParams.push_back(UserParam(ContactPtr_diff_string_
        //                                           +a_type));
        //b_a->params.userParams.push_back(UserParam(ContactPtr_diff_string_
        //                                     +b_type));
    }
}

PWIZ_API_DECL
void diff(const Affiliations& a,
          const Affiliations& b,
          Affiliations& a_b,
          Affiliations& b_a,
          const DiffConfig& config)
{
    if (a.organizationPtr.get() &&  b.organizationPtr.get())
        diff(a.organizationPtr, b.organizationPtr,
             a_b.organizationPtr, b_a.organizationPtr, config);
    else
    {
        a_b.organizationPtr = a.organizationPtr;
        b_a.organizationPtr = b.organizationPtr;
    }
}

void diff(const Person& a,
          const Person& b,
          Person& a_b,
          Person& b_a,
          const DiffConfig& config)
{
    diff((const Contact&)a, (const Contact&)b,
         (Contact&)a_b, (Contact&)b_a, config);
    diff(a.lastName, b.lastName, a_b.lastName, b_a.lastName, config);
    diff(a.firstName, b.firstName, a_b.firstName, b_a.firstName, config);
    diff(a.midInitials, b.midInitials, a_b.midInitials, b_a.midInitials,
         config);
    vector_diff_diff(a.affiliations, b.affiliations, a_b.affiliations,
                b_a.affiliations, config);
}

void diff(const PersonPtr a,
          const PersonPtr b,
          PersonPtr a_b,
          PersonPtr b_a,
          const DiffConfig& config)
{
    diff(*a, *b, *a_b, *b_a, config);
}

PWIZ_API_DECL
void diff(const Organization& a,
          const Organization& b,
          Organization& a_b,
          Organization& b_a,
          const DiffConfig& config)
{
    diff((const Contact&)a, (const Contact&)b,
         (Contact&)a_b, (Contact&)b_a, config);
    ptr_diff(a.parent.organizationPtr, b.parent.organizationPtr,
         a_b.parent.organizationPtr, b_a.parent.organizationPtr,
        config);
}


PWIZ_API_DECL
void diff(const BibliographicReference& a,
          const BibliographicReference& b,
          BibliographicReference& a_b,
          BibliographicReference& b_a,
          const DiffConfig& config)
{
    diff((const IdentifiableType&)a, (const IdentifiableType&)b,
         (IdentifiableType&)a_b, (IdentifiableType&)b_a,  config);
    diff(a.authors, b.authors, a_b.authors, b_a.authors, config);
    diff(a.publication, b.publication, a_b.publication, b_a.publication, config);
    diff(a.publisher, b.publisher, a_b.publisher, b_a.publisher, config);
    diff(a.editor, b.editor, a_b.editor, b_a.editor, config);
    diff_numeric(a.year, b.year, a_b.year, b_a.year, config);
    diff(a.volume, b.volume, a_b.volume, b_a.volume, config);
    diff(a.issue, b.issue, a_b.issue, b_a.issue, config);
    diff(a.pages, b.pages, a_b.pages, b_a.pages, config);
    diff(a.title, b.title, a_b.title, b_a.title, config);
}

PWIZ_API_DECL
void diff(const ProteinDetection& a,
          const ProteinDetection& b,
          ProteinDetection& a_b,
          ProteinDetection& b_a,
          const DiffConfig& config)
{
    ptr_diff(a.proteinDetectionProtocolPtr, b.proteinDetectionProtocolPtr,
         a_b.proteinDetectionProtocolPtr, b_a.proteinDetectionProtocolPtr,
         config);
    ptr_diff(a.proteinDetectionListPtr, b.proteinDetectionListPtr,
         a_b.proteinDetectionListPtr, b_a.proteinDetectionListPtr,
         config);
    diff(a.activityDate, b.activityDate, a_b.activityDate, b_a.activityDate,
         config);
    vector_diff_deep(a.inputSpectrumIdentifications,
                     b.inputSpectrumIdentifications,
                     a_b.inputSpectrumIdentifications,
                     b_a.inputSpectrumIdentifications,
                     config);
}

PWIZ_API_DECL
void diff(const SpectrumIdentification& a,
          const SpectrumIdentification& b,
          SpectrumIdentification& a_b,
          SpectrumIdentification& b_a,
          const DiffConfig& config)
{
    ptr_diff(a.spectrumIdentificationProtocolPtr,
         b.spectrumIdentificationProtocolPtr,
         a_b.spectrumIdentificationProtocolPtr,
         b_a.spectrumIdentificationProtocolPtr, config);
    ptr_diff(a.spectrumIdentificationListPtr,
         b.spectrumIdentificationListPtr,
         a_b.spectrumIdentificationListPtr,
         b_a.spectrumIdentificationListPtr, config);
    diff(a.activityDate, b.activityDate, a_b.activityDate,
         b_a.activityDate, config);
    vector_diff_diff(a.inputSpectra, b.inputSpectra, a_b.inputSpectra,
         b_a.inputSpectra, config);
    vector_diff_diff(a.searchDatabase, b.searchDatabase, a_b.searchDatabase,
         b_a.searchDatabase, config);
}

PWIZ_API_DECL
void diff(const AnalysisCollection& a,
          const AnalysisCollection& b,
          AnalysisCollection& a_b,
          AnalysisCollection& b_a,
          const DiffConfig& config)
{
    vector_diff_deep(a.spectrumIdentification, b.spectrumIdentification,
                     a_b.spectrumIdentification, b_a.spectrumIdentification,
                     config);
    diff(a.proteinDetection, b.proteinDetection, a_b.proteinDetection,
         b_a.proteinDetection, config);
}

PWIZ_API_DECL
void diff(const DBSequence& a,
          const DBSequence& b,
          DBSequence& a_b,
          DBSequence& b_a,
          const DiffConfig& config)
{
    diff((IdentifiableType&)a, (IdentifiableType&)b,
         (IdentifiableType&)a_b, (IdentifiableType&)b_a, config);
    diff_numeric(a.length, b.length, a_b.length, b_a.length, config);
    diff(a.accession, b.accession, a_b.accession, b_a.accession, config);
    ptr_diff(a.searchDatabasePtr, b.searchDatabasePtr, a_b.searchDatabasePtr,
         b_a.searchDatabasePtr, config);
    diff(a.seq, b.seq, a_b.seq, b_a.seq, config);
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup, config);
}

PWIZ_API_DECL
void diff(const Peptide& a,
          const Peptide& b,
          Peptide& a_b,
          Peptide& b_a,
          const DiffConfig& config)
{
    diff(a.peptideSequence, b.peptideSequence, a_b.peptideSequence,
         b_a.peptideSequence, config);
    vector_diff_deep(a.modification, b.modification, a_b.modification,
                     b_a.modification, config);
    diff(a.substitutionModification, b.substitutionModification,
         a_b.substitutionModification,b_a.substitutionModification, config);
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup, config);
}


PWIZ_API_DECL
void diff(const Modification& a,
          const Modification& b,
          Modification& a_b,
          Modification& b_a,
          const DiffConfig& config)
{
    diff_numeric(a.location, b.location, a_b.location, b_a.location, config);
    diff(a.residues, b.residues, a_b.residues, b_a.residues, config);
    diff_numeric(a.avgMassDelta, b.avgMassDelta, a_b.avgMassDelta, b_a.avgMassDelta, config);
    diff_numeric(a.monoisotopicMassDelta, b.monoisotopicMassDelta, a_b.monoisotopicMassDelta, b_a.monoisotopicMassDelta, config);
    diff(a.paramGroup, b.paramGroup, a_b.paramGroup, b_a.paramGroup, config);
}

PWIZ_API_DECL
void diff(const SubstitutionModification& a,
          const SubstitutionModification& b,
          SubstitutionModification& a_b,
          SubstitutionModification& b_a,
          const DiffConfig& config)
{
    diff(a.originalResidue, b.originalResidue, a_b.originalResidue,
         b_a.originalResidue, config);
    diff(a.replacementResidue, b.replacementResidue, a_b.replacementResidue,
         b_a.replacementResidue, config);
    diff_numeric(a.location, b.location, a_b.location, b_a.location, config);
    diff_numeric(a.avgMassDelta, b.avgMassDelta, a_b.avgMassDelta,
         b_a.avgMassDelta, config);
    diff_numeric(a.monoisotopicMassDelta, b.monoisotopicMassDelta,
         a_b.monoisotopicMassDelta, b_a.monoisotopicMassDelta, config);
}

PWIZ_API_DECL
void diff(const SequenceCollection& a,
          const SequenceCollection& b,
          SequenceCollection& a_b,
          SequenceCollection& b_a,
          const DiffConfig& config)
{
    vector_diff_deep(a.dbSequences, b.dbSequences, a_b.dbSequences,
                     b_a.dbSequences, config);
    
    vector_diff_deep(a.peptides, b.peptides, a_b.peptides, b_a.peptides,
                     config);
}

PWIZ_API_DECL
void diff(const Sample::subSample& a,
          const Sample::subSample& b,
          Sample::subSample& a_b,
          Sample::subSample& b_a,
          const DiffConfig& config)
{
    ptr_diff(a.samplePtr, b.samplePtr,
         a_b.samplePtr, b_a.samplePtr, config);
}

PWIZ_API_DECL
void diff(const Sample& a,
          const Sample& b,
          Sample& a_b,
          Sample& b_a,
          const DiffConfig& config)
{
    vector_diff_diff(a.subSamples, b.subSamples, a_b.subSamples,
                     b_a.subSamples, config);
}

PWIZ_API_DECL
void diff(const AnalysisSampleCollection& a,
          const AnalysisSampleCollection& b,
          AnalysisSampleCollection& a_b,
          AnalysisSampleCollection& b_a,
          const DiffConfig& config)
{
    vector_diff_deep(a.samples, b.samples, a_b.samples, b_a.samples, config);
}

PWIZ_API_DECL
void diff(const Provider& a,
          const Provider& b,
          Provider& a_b,
          Provider& b_a,
          const DiffConfig& config)
{
    diff((const IdentifiableType&)a, (const IdentifiableType&)b,
         (IdentifiableType&)a_b, (IdentifiableType&)b_a, config);
    diff(a.contactRole, b.contactRole, a_b.contactRole,
         b_a.contactRole, config);
}

PWIZ_API_DECL
void diff(const ContactRole& a,
          const ContactRole& b,
          ContactRole& a_b,
          ContactRole& b_a,
          const DiffConfig& config)
{
    diff(a.contactPtr, b.contactPtr, a_b.contactPtr, b_a.contactPtr, config);
    diff(a.role, b.role, a_b.role, b_a.role, config);
}

PWIZ_API_DECL
void diff(const AnalysisSoftwarePtr a,
          const AnalysisSoftwarePtr b,
          AnalysisSoftwarePtr a_b,
          AnalysisSoftwarePtr b_a,
          const DiffConfig& config)
{
    ptr_diff(a, b, a_b, b_a, config);
}


PWIZ_API_DECL
void diff(const AnalysisSoftware& a,
          const AnalysisSoftware& b,
          AnalysisSoftware& a_b,
          AnalysisSoftware& b_a,
          const DiffConfig& config)
{
    diff((const IdentifiableType&)a, (const IdentifiableType&)b,
         (IdentifiableType&)a_b, (IdentifiableType&)b_a, config);
    diff(a.version, b.version, a_b.version, b_a.version, config);
    ptr_diff(a.contactRolePtr, b.contactRolePtr, a_b.contactRolePtr, b_a.contactRolePtr, config);
    diff(a.softwareName, b.softwareName, a_b.softwareName, b_a.softwareName, config);
    diff(a.URI, b.URI, a_b.URI, b_a.URI, config);
    diff(a.customizations, b.customizations, a_b.customizations, b_a.customizations, config);
}


PWIZ_API_DECL
void diff(const MzIdentML& a, 
          const MzIdentML& b, 
          MzIdentML& a_b, 
          MzIdentML& b_a,
          const DiffConfig& config)
{
    // Attributes
    diff((const IdentifiableType&)a, (const IdentifiableType&)b,
         (IdentifiableType&)a_b, (IdentifiableType&)b_a, config);
    diff(a.version, b.version, a_b.version, b_a.version, config);
    diff(a.creationDate, b.creationDate, a_b.creationDate, b_a.creationDate, config);

    // Elements
    vector_diff_diff(a.cvs, b.cvs, a_b.cvs, b_a.cvs, config);
    vector_diff_deep(a.analysisSoftwareList, b.analysisSoftwareList, a_b.analysisSoftwareList, b_a.analysisSoftwareList, config);
    diff(a.provider, b.provider, a_b.provider, b_a.provider, config);
    vector_diff_deep(a.auditCollection, b.auditCollection, a_b.auditCollection, b_a.auditCollection, config);
    diff(a.analysisSampleCollection, b.analysisSampleCollection,
         a_b.analysisSampleCollection, b_a.analysisSampleCollection, config);
    diff(a.sequenceCollection, b.sequenceCollection,
         a_b.sequenceCollection, b_a.sequenceCollection, config);
    diff(a.analysisCollection, b.analysisCollection, a_b.analysisCollection, b_a.analysisCollection, config);
    diff(a.analysisProtocolCollection, b.analysisProtocolCollection, a_b.analysisProtocolCollection, b_a.analysisProtocolCollection, config);
    diff(a.dataCollection, b.dataCollection, a_b.dataCollection, b_a.dataCollection, config);
    vector_diff_deep(a.bibliographicReference, b.bibliographicReference, a_b.bibliographicReference, b_a.bibliographicReference, config);
    

    
    // provide names for context
    //if (!a_b.empty() && a_b.name.empty()) a_b.name = a.name; 
    //if (!b_a.empty() && b_a.name.empty()) b_a.name = b.name; 
}

PWIZ_API_DECL
void diff(const IdentifiableType& a,
          const IdentifiableType& b,
          IdentifiableType& a_b,
          IdentifiableType& b_a,
          const DiffConfig& config)
{
    diff(a.id, b.id, a_b.id, b_a.id, config);
    diff(a.name, b.name, a_b.name, b_a.name, config);
}

} // namespace diff_impl

PWIZ_API_DECL
std::ostream& operator<<(std::ostream& os, const Diff<MzIdentML>& diff)
{
  using namespace diff_impl;

  TextWriter write(os,1);

  if(!diff.a_b.empty() || !diff.b_a.empty())
  {
      os<<"+\n";
      write(diff.a_b);
      os<<"-\n";
      write(diff.b_a);
  }

    return os;

}

    
} // namespace mziddata
} // namespace pwiz
