#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "choices.hpp"
#include "feature.hpp"
#include "model.hpp"
#include "polymer.hpp"
#include "reaction.hpp"
#include "tracker.hpp"

namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(core, m) {
  m.doc() = (R"doc(
    Python module
    -----------------------
    .. currentmodule:: pinetree
  )doc");

  py::class_<BindingSite, std::shared_ptr<BindingSite>>(m, "BindingSite",
                                                        R"doc(
            BindingSite class that corresponds to both promoters and ribosome 
            binding sites. For internal use only.

            )doc")
      .def(py::init<std::string, int, int, std::map<std::string, double>>())
      .def("reset_state", &BindingSite::ResetState)
      .def("was_uncovered", &BindingSite::WasUncovered)
      .def("was_covered", &BindingSite::WasCovered)
      .def("cover", &BindingSite::Cover)
      .def("uncover", &BindingSite::Uncover)
      .def("is_covered", &BindingSite::IsCovered)
      .def("clone", &BindingSite::Clone)
      .def("check_interaction", &BindingSite::CheckInteraction)
      .def_property(
          "first_exposure",
          (bool (BindingSite::*)(void) const) & BindingSite::first_exposure,
          (void (BindingSite::*)(bool)) & BindingSite::first_exposure);

  py::class_<ReleaseSite, std::shared_ptr<ReleaseSite>>(m, "ReleaseSite",
                                                        R"doc(
            ReleaseSite class that corresponds to both terminators and stop codons. For internal use only.

            )doc")
      .def(py::init<std::string, int, int, std::map<std::string, double>>())
      .def("reset_state", &ReleaseSite::ResetState)
      .def("was_uncovered", &ReleaseSite::WasUncovered)
      .def("was_covered", &ReleaseSite::WasCovered)
      .def("cover", &ReleaseSite::Cover)
      .def("uncover", &ReleaseSite::Uncover)
      .def("is_covered", &ReleaseSite::IsCovered)
      .def("clone", &ReleaseSite::Clone)
      .def("check_interaction", &ReleaseSite::CheckInteraction)
      .def_property(
          "readthrough",
          (bool (ReleaseSite::*)(void) const) & ReleaseSite::readthrough,
          (void (ReleaseSite::*)(bool)) & ReleaseSite::readthrough)
      .def("efficiency", &ReleaseSite::efficiency);

  py::class_<Polymerase, std::shared_ptr<Polymerase>>(m, "Polymerase",
                                                      R"doc(
            Polymerase class that corresponds to both biological polymerases and ribosomes. For internal use only.

            )doc")
      .def(py::init<std::string, int, int>())
      .def("move", static_cast<void (Polymerase::*)()>(&Polymerase::Move))
      .def("move_back", static_cast<void (Polymerase::*)()>(&Polymerase::MoveBack))
      .def_property("start",
                    (int (Polymerase::*)(void) const) & Polymerase::start,
                    (void (Polymerase::*)(int)) & Polymerase::start)
      .def_property("stop",
                    (int (Polymerase::*)(void) const) & Polymerase::stop,
                    (void (Polymerase::*)(int)) & Polymerase::stop)
      .def_property_readonly(
          "speed", (int (Polymerase::*)(void) const) & Polymerase::speed)
      .def_property_readonly("footprint", (int (Polymerase::*)(void) const) &
                                              Polymerase::footprint)
      .def_property(
          "reading_frame",
          (int (Polymerase::*)(void) const) & Polymerase::reading_frame,
          (void (Polymerase::*)(int)) & Polymerase::reading_frame)
      .def_property(
          "polymerase_read_through",
          (bool (Polymerase::*)(void)) & Polymerase::polymerasereadthrough,
          (void (Polymerase::*)(bool)) & Polymerase::polymerasereadthrough);    
  py::class_<Mask, std::shared_ptr<Mask>>(m, "Mask",
                                          R"doc(
            Mask class that corresponds to polymers that are still undergoing 
            synthesis. For internal use only.

            )doc")
      .def(py::init<int, int, std::map<std::string, double>>())
      .def("move", &Mask::Move)
      .def("move_back", &Mask::MoveBack)
      .def_property("start", (int (Mask::*)(void) const) & Mask::start,
                    (void (Mask::*)(int)) & Mask::start)
      .def_property("stop", (int (Mask::*)(void) const) & Mask::stop,
                    (void (Mask::*)(int)) & Mask::stop)
      .def_property_readonly("speed", (int (Mask::*)(void) const) & Mask::speed)
      .def_property_readonly("footprint",
                             (int (Mask::*)(void) const) & Mask::footprint)
      .def_property("reading_frame",
                    (int (Mask::*)(void) const) & Mask::reading_frame,
                    (void (Mask::*)(int)) & Mask::reading_frame)
      .def("check_interaction", &Mask::CheckInteraction);

  py::class_<Rnase, std::shared_ptr<Rnase>>(m, "Rnase",
                                            R"doc(
            Rnase class that corresponds to polymers that are being degraded 
            from 5' to 3' end. For internal use only.

            )doc")
      .def(py::init<int, int>())
      .def("move", &Rnase::Move)
      .def("move_back", &Rnase::MoveBack)
      .def_property("start", (int (Rnase::*)(void) const) & Rnase::start,
                    (void (Rnase::*)(int)) & Rnase::start)
      .def_property("stop", (int (Rnase::*)(void) const) & Rnase::stop,
                    (void (Rnase::*)(int)) & Rnase::stop)
      .def_property_readonly("speed",
                             (int (Rnase::*)(void) const) & Rnase::speed)
      .def_property_readonly("footprint",
                             (int (Rnase::*)(void) const) & Rnase::footprint)
      .def_property("reading_frame",
                    (int (Rnase::*)(void) const) & Rnase::reading_frame,
                    (void (Rnase::*)(int)) & Rnase::reading_frame);

  py::class_<SpeciesReaction, std::shared_ptr<SpeciesReaction>>(
      m, "SpeciesReaction",
      R"doc(
            
            Defines reactions between two or fewer species (with stoichiometries
             of 1). For internal use only.
            
            )doc")
      .def(py::init<double, double, std::vector<std::string>,
                    std::vector<std::string>>())
      .def("caculate_propensity", &SpeciesReaction::CalculatePropensity)
      .def("execute", &SpeciesReaction::Execute)
      .def_property_readonly(
          "reactants",
          (std::vector<std::string>(SpeciesReaction::*)(void) const) &
              SpeciesReaction::reactants)
      .def_property_readonly(
          "products",
          (std::vector<std::string>(SpeciesReaction::*)(void) const) &
              SpeciesReaction::products);

  py::class_<MobileElementManager, std::shared_ptr<MobileElementManager>>(
      m, "MobileElementManager",
      R"doc(

      Manages MobileElements (polymerases, ribosomes, RNases) on a Polymer. For
      internal use only.
  
      [])doc")
      .def(py::init())
      .def("insert", &MobileElementManager::Insert)
      .def("delete", &MobileElementManager::Delete)
      .def("choose", &MobileElementManager::Choose)
      .def("valid_index", &MobileElementManager::ValidIndex)
      .def("get_pol", &MobileElementManager::GetPol)
      .def("get_attached", &MobileElementManager::GetAttached)
      .def("update_propensity", &MobileElementManager::UpdatePropensity)
      .def_property_readonly("prop_sum",
                             (double (MobileElementManager::*)(void)) &
                                 MobileElementManager::prop_sum)
      .def_property_readonly("pol_count",
                             (int (MobileElementManager::*)(void)) &
                                 MobileElementManager::pol_count);

  py::class_<Model, std::shared_ptr<Model>>(m, "Model",
                                            R"doc(
            
            Define a pinetree model.
            
            Args:
                cell_volume (float): The volume, in liters, of the system being simulated.
             
            Examples:

                >>> import pinetree.pinetree as pt
                >>> sim = pt.Model(cell_volume=8e-16) # Approximate volume of E. coli cell

           )doc")
      .def(py::init<double>(), "cell_volume"_a)
      .def("seed", &Model::seed,
           R"doc(
             
             Set a seed for reproducible simulations.
             
             Args:
                seed (int): a seed for the random number generator

             )doc")
      .def("add_reaction", &Model::AddReaction, "rate_constant"_a,
           "reactants"_a, "products"_a, R"doc(
             
            Define a reaction between species, which may include free 
            ribosomes and polymerases.

            Args:
                rate_constant (float): Macroscopic rate constant of the 
                    reaction. This will be converted into a stochastic 
                    mesoscopic rate constant automatically.
                reactants (list): List of reactants, which may be species, 
                    ribosomes, or polymerases
                products (list): List of products which may be species, 
                    ribosomes, or polymerases
        
            Note:
                Reaction rate constants should be given as macroscopic rate 
                constants, the same constants used in differential 
                equation-based models. Pinetree will automatically 
                convert these rate constants to mesoscopic constants required 
                for a stochastic simulation.
            
            Example:

                >>> coming soon
             
           )doc")
      .def("add_species", &Model::AddSpecies, "name"_a, "copy_number"_a,
           R"doc(
           
           Defines individual chemical species not specified by either ``add_ribosome()`` or ``add_polymerase()``.

           Args:
              name (str): Name of chemical species which can be referred to in    reactions added with ``add_reaction()``.
              copy_number (int): Initial number of copies of the chemical species 

           )doc")
      .def("add_polymerase", &Model::AddPolymerase, "name"_a, "footprint"_a,
           "speed"_a, "copy_number"_a, R"doc(

           Add a polymerase to the model. There may be multiple types of
           polymerases in a model.

           .. note::
              
              Defining a polymerase with a footprint larger than that of the 
              promoter it binds is not currently supported.
           
           Args:
              name (str): Name of the polymerase which can be referred to in 
                  ``add_reaction()`` and ``add_promoter()``.
              copy_number (int): Initial number of copies of the polymerase
              speed (int): Speed, in base pairs per second, at which the 
                  polymerase transcribes
              footprint (int): Footprint, in base pairs, of the polymerase on 
                  the genome

           )doc")
      .def("add_polymerase_with_readthrough", &Model::AddPolymeraseWithReadthrough, "name"_a, "footprint"_a,
           "speed"_a, "copy_number"_a, R"doc(

           Add a polymerase with genome end readthrough to the model. This is 
           used to simulate transcription of a circular genome. 

           .. note::
              
              Defining a polymerase with a footprint larger than that of the 
              promoter it binds is not currently supported.
           
           Args:
              name (str): Name of the polymerase which can be referred to in 
                  ``add_reaction()`` and ``add_promoter()``.
              copy_number (int): Initial number of copies of the polymerase
              speed (int): Speed, in base pairs per second, at which the 
                  polymerase transcribes
              footprint (int): Footprint, in base pairs, of the polymerase on 
                  the genome

           )doc")
      .def("add_trna", (void (Model::*)(std::map<std::string, std::vector<std::string>>&, std::map<std::string, std::pair<int, int>>&, std::map<std::string, double>&)) &Model::AddtRNA, "codon_map"_a, "counts"_a, "rate_constants"_a, 
           R"doc(

           Simulate translation with dynamic tRNAs. 

           .. note::
              
              This feature is currently experimental and may produce unexpected results.

           Makes translation dependent on tRNA abundances, by modifing the underlying propensity calculation for 
           ribosome movement (specifically, the movement propensity for each ribosome gets multiplied by the number 
           of available charged tRNAs that correspond to its occupied codon). Rate constants for the tRNA re-charging 
           reaction can be set for each tRNA species independently. 
           
           Adding dynamic tRNAs is likely most useful for simulating codon usage bias, or for modeling situations 
           where tRNA pools are extremely uneven.

           .. warning::
              
              Simulations with tRNA dynamics turned on can be very slow. 

           Args:
              codon_map (dict): Specifies the codon:tRNA mapping (note that one codon 
                  can map to multple tRNAs) 
              counts (dict): Initial charged and uncharged counts for all tRNA species,
                  specified as ``tRNA_name: [<charged count>, <uncharged count>]``
              rate_constants (dict): Charging rate constants for each tRNA species.
            
           Examples:

                >>> sim = pt.Model(cell_volume=8e-16)
                >>>
                >>> tRNA_map = {"AAA": ["TTT", "TTG"], "TAT": ["ATA"]}
                >>> tRNA_counts = {"TTT": [250, 0], "TTG": [50, 0], "ATA": [250, 0]}
                >>> tRNA_rates = {"TTT": 100, "ATA": 100, "TTG": 10}
                >>> sim.add_trna(tRNA_map, tRNA_counts, tRNA_rates)

           )doc")
      .def("add_ribosome", &Model::AddRibosome, "footprint"_a, "speed"_a,
           "copy_number"_a, R"doc(

           Add ribosomes to the model. There may only be a single type of 
           ribosome.

           .. note::
              
              Defining ribosomes with a footprint larger than that of the 
              promoter it binds is not currently supported.
           
           Args:
              copy_number (int): Initial number of copies of free ribosomes
              speed (double): Mean speed, in base pairs per second, at which the 
                  ribosome translates. This speed will be scaled on a per site
                  basis if translation weights are defined. (See 
                  Genome.AddWeights).
              footprint (int): Footprint, in base pairs, of the ribosome on RNA

           )doc")
      .def("register_genome", &Model::RegisterGenome, R"doc(
        
        Register a genome with the model.

        Args:
            genome (Genome): a pinetree ``Genome`` object.
        
        )doc")
      .def("register_transcript", &Model::RegisterTranscript, R"doc(
        
        Register a genome-independent transcript with the model.

        Args:
            transcript (Transcript): a pinetree ``Transcript`` object.
        
        )doc")
      .def("simulate", &Model::Simulate, "time_limit"_a, "time_step"_a,
           "output"_a = "counts.tsv",
           R"doc(
            
            Run a gene expression simulation. Produces a tab separated file of 
            protein and transcript counts at user-specified time intervals.

            Args:
                time_limit (int): Simulated time, in seconds at which this 
                    simulation should stop executing reactions. Note that this 
                    *simulated* time and not real time. The real time that it 
                    takes for the simulation to complete depends on the number 
                    of reactions and species (genomes, transcripts, proteins, 
                    etc) in the system.
                time_step (double): Time interval, in seconds, that species counts 
                    are reported.
                output (str): Name of output file (default: counts.tsv).

          )doc");

  // Polymers, genomes, and transcripts
  py::class_<Polymer, Polymer::Ptr>(m, "Polymer");
  py::class_<Genome, Polymer, Genome::Ptr>(m, "Genome")
      .def(py::init<const std::string &, int, double, double, int, double>(),
           "name"_a, "length"_a, "transcript_degradation_rate_ext"_a = 0.0, 
           "rnase_speed"_a = 0.0, "rnase_footprint"_a = 0,
           "transcript_degradation_rate"_a = 0.0,
           R"doc(
            
            Define a linear genome. 

            Args:
                name (str): Name of genome.
                length (int): Length of genome in base pairs.
                transcript_degradation_rate_ext (float): Unary binding rate 
                    constant for binding of RNases to the 5'-end of transcripts.
                rnase_speed (flaot): Mean speed at which RNase degrades 
                    transcript, in bases per second.
                rnase_footprint (float): Initial footprint of RNase on RNA.
                transcript_degradation_rate (float): Deprecated. Unary binding rate constant for 
                    binding of RNases to internal RNase sites. (See the add_rnase_site() 
                    method below for details.)

          )doc")
      .def("add_mask", &Genome::AddMask, "start"_a, "interactions"_a,
           R"doc(
            
            Mask a portion of this Genome. This mask may correspond to a portion
            of the genome that has not yet entered the cell or is otherwise 
            inaccessible. Also define which Polymerases are capabile of moving 
            the Mask (e.g. an RNA polymerase that actively pulls the genome 
            into the cell.)

            Args:
                start (int): Start position of Mask. The Mask is assumed to 
                    extend to the end of the genome.
                interactions (list): List of Polymerase names capable of 
                    shifting the Mask backwards and revealing more of the 
                    genome.
            
            )doc")
      .def("add_sequence", &Genome::AddSequence, "seq"_a,
           R"doc(
            
            Define the nucleotide sequence for the genome being simulated. 

            .. note::
              
              This is required for simulations with tRNAs.

            Args:
                seq (string): The nucleotide sequence. This should be the same
                    length as Genome.

            )doc")
      .def("add_weights", &Genome::AddWeights, "weights"_a,
           R"doc(
            
            Add translation weights for this genome. Can be used to specify codon-specific 
            translation speeds.

            .. note::
              
              Using weights in combination with tRNA tracking (with ``Model.add_trna``)
              is not recommended. In the event that both are specified, the model will 
              preferrentially try to use tRNA abundances to calculate ribosome speeds, 
              essentially ignoring whatever ``weights`` is set to. 

            Args:
                weights (list): List of weights of same length as Genome. These
                    weights are multiplied by the ribosome speed to calculate a 
                    final translation rate at every position in the genome.

            )doc")
      .def("add_promoter", &Genome::AddPromoter, "name"_a, "start"_a, "stop"_a,
           "interactions"_a,
           R"doc(
            
            Define a promoter.

            Args:
                name (str): Name of promoter.
                start (int): Start position of promoter.
                stop (int): Stop position of promoter.
                interactions (dict): Dictionary of binding rate constants for
                    different Polymerases that this promoter interacts with.
            
            Example:
                
                >>> genome.add_promoter(name="phi1", start=1, stop=10,
                >>>                     interactions={'rnapol': 1e7})

            )doc")
      .def("add_terminator", &Genome::AddTerminator, "name"_a, "start"_a,
           "stop"_a, "efficiency"_a,
           R"doc(
            
            Define a terminator.

            Args:
                name (str): Name of terminator.
                start (int): Start position of terminator.
                stop (int): Stop position of terminator.
                efficiency (dict): Dictionary of termination efficiencies 
                    (between 0 and 1) for different Polymerases that this 
                    terminator interacts with. A value of 1.0 represents 
                    complete stop of transcription and removal of the 
                    Polymerase. A value of 0.0 means that the Polymerase will
                    always read through the terminator and continue 
                    transcription.
            
            Example:
                
                >>> genome.add_terminator(name="t1", start=50, stop=51,
                >>>                       efficiency={'rnapol': 0.85})

            )doc")
      .def("add_gene", &Genome::AddGene, "name"_a, "start"_a, "stop"_a,
           "rbs_start"_a, "rbs_stop"_a, "rbs_strength"_a,
           R"doc(
            
            Define a gene. Genes may be defined in any order.

            .. note::
               
               Overlapping genes, or genes that overlap with 
               ribosome binding sites, are supported as of release
               0.3.0. If using an ealier version, specifing overlapping
               genes is not recommended, and could cause the program
               to crash.

            Args:
                name (str): Name of gene. Name may be referenced by 
                    ``Genome.add_reaction``.
                start (int): Start position of gene.
                stop (int): Stop position of gene.
                rbs_start (int): Start position of ribosome binding site.       
                    Generally positioned upstream of gene start.
                rbs_stop (int): Stop position of ribosome binding site.
                rbs_strength (float): Binding rate constant between ribosome
                    and ribosome binding site.

            )doc")
      .def("add_rnase_site", (void (Genome::*)(const std::string&, int, int, double)) &Genome::AddRnaseSite, 
           "name"_a, "start"_a, "stop"_a, "rate"_a)
      .def("add_rnase_site", (void (Genome::*)(int, int)) &Genome::AddRnaseSite, "start"_a, "stop"_a);
  py::class_<Transcript, Polymer, Transcript::Ptr>(m, "Transcript")
      .def(py::init<const std::string &, int>(), "name"_a, "length"_a,
           R"doc(
            
            Define a linear transcript with one or more genes. These 
            transcripts cannot be degraded. 

            Args:
                name (str): Name of transcript.
                length (int): Length of transcript in base pairs.

          )doc")
      .def("add_gene", &Transcript::AddGene, "name"_a, "start"_a, "stop"_a,
           "rbs_start"_a, "rbs_stop"_a, "rbs_strength"_a,
           R"doc(
            
            Define a gene. Genes may be defined in any order.

            .. note::
               
               At this time, overlapping genes or genes that overlap with 
               ribosome binding sites are not supported.

            Args:
                name (str): Name of gene. Name may be referenced by 
                    ``Genome.add_reaction``.
                start (int): Start position of gene.
                stop (int): Stop position of gene.
                rbs_start (int): Start position of ribosome binding site.       
                    Generally positioned upstream of gene start.
                rbs_stop (int): Stop position of ribosome binding site.
                rbs_strength (float): Binding rate constant between ribosome
                    and ribosome binding site.

            )doc")
      .def("add_seq", &Transcript::AddSequence, "seq"_a,
           R"doc(
            
            Define a nucleotide sequence for this transcript. Note that this should be 
            the DNA sequence corresponding to the parent gene(s) (not the transcribed
            RNA sequence).

            Args:
                seq (string): The nucleotide sequence for this transcript. Should be the same length 
                    as Transcript.

            )doc")
      .def("add_weights", &Transcript::AddWeights, "weights"_a,
           R"doc(
            
            Define position-specific translation speed weights. These may correspond, for example, codon-specific 
            translation rates. See also ``Genome.add_weights``.

            Args:
                weights (list): List of weights of same length as Transcript. These weights are multiplied by 
                    the ribosome speed to calculate a final translation rate at every position in the genome.

            )doc");
}