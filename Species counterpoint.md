Species counterpoint

\new PianoStaff <<
  \new Staff \relative c' { d4 e f g | a b c d | e d c b | a b c a | g a b cis | d1 | }
  \new Staff \relative c' { d1 | f | g | f | e | d | \bar "|." }
>>
Example of "third species" counterpoint
Species counterpoint was developed as a pedagogical tool in which students progress through several "species" of increasing complexity, with a very simple part that remains constant known as the cantus firmus (Latin for "fixed melody"). Species counterpoint generally offers less freedom to the composer than other types of counterpoint and therefore is called a "strict" counterpoint. The student gradually attains the ability to write free counterpoint (that is, less rigorously constrained counterpoint, usually without a cantus firmus) according to the given rules at the time.[17] The idea is at least as old as 1532, when Giovanni Maria Lanfranco described a similar concept in his Scintille di musica (Brescia, 1533). The 16th-century Venetian theorist Zarlino elaborated on the idea in his influential Le institutioni harmoniche, and it was first presented in a codified form in 1619 by Lodovico Zacconi in his Prattica di musica. Zacconi, unlike later theorists, included a few extra contrapuntal techniques, such as invertible counterpoint.


Gradus ad Parnassum (1725) by Johann Joseph Fux defines the modern system of teaching counterpoint
In 1725 Johann Joseph Fux published Gradus ad Parnassum (Steps to Parnassus), in which he described five species:

Note against note;
Two notes against one;
Four notes against one;
Notes offset against each other (as suspensions);
All the first four species together, as "florid" counterpoint.
A succession of later theorists quite closely imitated Fux's seminal work, often with some small and idiosyncratic modifications in the rules. Many of Fux's rules concerning the purely linear construction of melodies have their origin in solfeggio. Concerning the common practice era, alterations to the melodic rules were introduced to enable the function of certain harmonic forms. The combination of these melodies produced the basic harmonic structure, the figured bass.[citation needed]

Considerations for all species
The following rules apply to melodic writing in each species, for each part:

The final note must be approached by step. If the final is approached from below, then the leading tone must be raised in a minor key (Dorian, Hypodorian, Aeolian, Hypoaeolian), but not in Phrygian or Hypophrygian mode. Thus, in the Dorian mode on D, a C♯ is necessary at the cadence.[18]
Permitted melodic intervals are the perfect unison, fourth, fifth, and octave, as well as the major and minor second, major and minor third, and ascending minor sixth. The ascending minor sixth must be immediately followed by motion downwards.
If writing two skips in the same direction—something that must be only rarely done—the second must be smaller than the first, and the interval between the first and the third note may not be dissonant. The three notes should be from the same triad; if this is impossible, they should not outline more than one octave. In general, do not write more than two skips in the same direction.
If writing a skip in one direction, it is best to proceed after the skip with step-wise motion in the other direction.
The interval of a tritone in three notes should be avoided (for example, an ascending melodic motion F–A–B♮)[19] as is the interval of a seventh in three notes.
There must be a climax or high point in the line countering the cantus firmus. This usually occurs somewhere in the middle of exercise and must occur on a strong beat.
An outlining of a seventh is avoided within a single line moving in the same direction.
And, in all species, the following rules govern the combination of the parts:

The counterpoint must begin and end on a perfect consonance.
Contrary motion should dominate.
Perfect consonances must be approached by oblique or contrary motion.
Imperfect consonances may be approached by any type of motion.
The interval of a tenth should not be exceeded between two adjacent parts unless by necessity.
Build from the bass, upward.
First species
In first species counterpoint, each note in every added part (parts being also referred to as lines or voices) sounds against one note in the cantus firmus. Notes in all parts are sounded simultaneously, and move against each other simultaneously. Since all notes in First species counterpoint are whole notes, rhythmic independence is not available.[20]

In the present context, a "step" is a melodic interval of a half or whole step. A "skip" is an interval of a third or fourth. (See Steps and skips.) An interval of a fifth or larger is referred to as a "leap".

A few further rules given by Fux, by study of the Palestrina style, and usually given in the works of later counterpoint pedagogues,[21] are as follows.


\relative c'' {
  <<
    \new Staff { \clef "treble" d1 a b d cis d }
    \new Staff { \clef "treble" d,1 f g f e d }
  >>
}
Short example of "first species" counterpoint
Begin and end on either the unison, octave, or fifth, unless the added part is underneath, in which case begin and end only on unison or octave.
Use no unisons except at the beginning or end.
Avoid parallel fifths or octaves between any two parts; and avoid "hidden" parallel fifths or octaves: that is, movement by similar motion to a perfect fifth or octave, unless one part (sometimes restricted to the higher of the parts) moves by step.
Avoid moving in parallel fourths. (In practice Palestrina and others frequently allowed themselves such progressions, especially if they do not involve the lowest of the parts.)
Do not use an interval more than three times in a row.
Attempt to use up to three parallel thirds or sixths in a row.
Attempt to keep any two adjacent parts within a tenth of each other, unless an exceptionally pleasing line can be written by moving outside that range.
Avoid having any two parts move in the same direction by skip.
Attempt to have as much contrary motion as possible.
Avoid dissonant intervals between any two parts: major or minor second, major or minor seventh, any augmented or diminished interval, and perfect fourth (in many contexts).
In the adjacent example in two parts, the cantus firmus is the lower part. (The same cantus firmus is used for later examples also. Each is in the Dorian mode.)

Second species
In second species counterpoint, two notes in each of the added parts work against each longer note in the given part.

 {
# (set-global-staff-size 15)
\relative c' {
<< \new Staff {
r2 a' d c b e d a b cis d1
\bar "|." }
\new Staff {
d,1 f g f e d} >>
}
}

Short example of "second species" counterpoint
Additional considerations in second species counterpoint are as follows, and are in addition to the considerations for first species:

It is permissible to begin on an upbeat, leaving a half-rest in the added voice.
The accented beat must have only consonance (perfect or imperfect). The unaccented beat may have dissonance, but only as a passing tone, i.e. it must be approached and left by step in the same direction.
Avoid the interval of the unison except at the beginning or end of the example, except that it may occur on the unaccented portion of the bar.
Use caution with successive accented perfect fifths or octaves. They must not be used as part of a sequential pattern. The example shown is weak due to similar motion in the second measure in both voices. A good rule to follow: if one voice skips or jumps try to use step-wise motion in the other voice or at the very least contrary motion.
Third species
 {
# (set-global-staff-size 16)
\relative c' {
\new PianoStaff <<
\new Staff {
d e f g a b c d
e d c b a b c a g a b cis d1
\bar "|." }
\new Staff {
d,1 f g f e d
}
>>
}
}

Short example of "third species" counterpoint
In third species counterpoint, four (or three, etc.) notes move against each longer note in the given part.

Three special figures are introduced into third species and later added to fifth species, and ultimately outside the restrictions of species writing. There are three figures to consider: The nota cambiata, double neighbor tones, and double passing tones.

Double neighbor tones: the figure is prolonged over four beats and allows special dissonances. The upper and lower tones are prepared on beat 1 and resolved on beat 4. The fifth note or downbeat of the next measure should move by step in the same direction as the last two notes of the double neighbor figure. Lastly a double passing tone allows two dissonant passing tones in a row. The figure would consist of 4 notes moving in the same direction by step. The two notes that allow dissonance would be beat 2 and 3 or 3 and 4. The dissonant interval of a fourth would proceed into a diminished fifth and the next note would resolve at the interval of a sixth.[18]


Example of a double passing tone in which the two middle notes are a dissonant interval from the cantus firmus, a fourth and a diminished fifth

Example of a descending double neighbor figure against a cantus firmus

Example of an ascending double neighbor figure (with an interesting tritone leap at the end) against a cantus firmus
Fourth species
In fourth species counterpoint, some notes are sustained or suspended in an added part while notes move against them in the given part, often creating a dissonance on the beat, followed by the suspended note then changing (and "catching up") to create a subsequent consonance with the note in the given part as it continues to sound. As before, fourth species counterpoint is called expanded when the added-part notes vary in length among themselves. The technique requires chains of notes sustained across the boundaries determined by beat, and so creates syncopation. A dissonant interval is allowed on beat 1 because of the syncopation created by the suspension. While it is not incorrect to start with a half note, it is also common to start 4th species with a half rest.


\relative c' {
\new PianoStaff <<
\new Staff {
\set Staff.explicitKeySignatureVisibility = #all-invisible
a'2 d~ d c~ c bes~
\key d \minor bes
a b cis d1 \bar "|."
}
\new Staff {
d, f g f e d \bar "|."
}
>>
}
Short example of "fourth species" counterpoint

Fifth species (florid counterpoint)
In fifth species counterpoint, sometimes called florid counterpoint, the other four species of counterpoint are combined within the added parts. In the example, the first and second bars are second species, the third bar is third species, the fourth and fifth bars are third and embellished fourth species, and the final bar is first species. In florid counterpoint it is important that no one species dominates the composition.


\relative c' {
\new PianoStaff <<
\new Staff {
r2 a' d c b4 c d e f e d2~ d4 cis8 b cis2 d1 \bar "|."
}
\new Staff {
d, f g f e d \bar "|."
}
>>
}
Short example of "Florid" counterpoint

Contrapuntal derivations
Since the Renaissance period in European music, much contrapuntal music has been written in imitative counterpoint. In imitative counterpoint, two or more voices enter at different times, and (especially when entering) each voice repeats some version of the same melodic element. The fantasia, the ricercar, and later, the canon and fugue (the contrapuntal form par excellence) all feature imitative counterpoint, which also frequently appears in choral works such as motets and madrigals. Imitative counterpoint spawned a number of devices, including:

Melodic inversion
The inverse of a given fragment of melody is the fragment turned upside down—so if the original fragment has a rising major third (see interval), the inverted fragment has a falling major (or perhaps minor) third, etc. (Compare, in twelve-tone technique, the inversion of the tone row, which is the so-called prime series turned upside down.) (Note: in invertible counterpoint, including double and triple counterpoint, the term inversion is used in a different sense altogether. At least one pair of parts is switched, so that the one that was higher becomes lower. See Inversion in counterpoint; it is not a kind of imitation, but a rearrangement of the parts.)
Retrograde
Whereby an imitative voice sounds the melody backwards in relation to the leading voice.
Retrograde inversion
Where the imitative voice sounds the melody backwards and upside-down at once.
Augmentation
When in one of the parts in imitative counterpoint the note values are extended in duration compared to the rate at which they were sounded when introduced.
Diminution
When in one of the parts in imitative counterpoint the note values are reduced in duration compared to the rate at which they were sounded when introduced.
Free counterpoint
Broadly speaking, due to the development of harmony, from the Baroque period on, most contrapuntal compositions were written in the style of free counterpoint. This means that the general focus of the composer had shifted away from how the intervals of added melodies related to a cantus firmus, and more toward how they related to each other.[citation needed][22]

Nonetheless, according to Kent Kennan: "....actual teaching in that fashion (free counterpoint) did not become widespread until the late nineteenth century."[23] Young composers of the eighteenth and nineteenth centuries, such as Mozart, Beethoven, and Schumann, were still educated in the style of "strict" counterpoint, but in practice, they would look for ways to expand on the traditional concepts of the subject.[citation needed]

Main features of free counterpoint:

All forbidden chords, such as second-inversion, seventh, ninth etc., can be used freely as long as they resolve to a consonant triad
Chromaticism is allowed
The restrictions about rhythmic-placement of dissonance are removed. It is possible to use passing tones on the accented beat
Appoggiatura is available: dissonance tones can be approached by leaps.
Linear counterpoint
[example needed]

Linear counterpoint is "a purely horizontal technique in which the integrity of the individual melodic lines is not sacrificed to harmonic considerations. "Its distinctive feature is rather the concept of melody, which served as the starting-point for the adherents of the 'new objectivity' when they set up linear counterpoint as an anti-type to the Romantic harmony."[3] The voice parts move freely, irrespective of the effects their combined motions may create."[24] In other words, either "the domination of the horizontal (linear) aspects over the vertical"[25] is featured or the "harmonic control of lines is rejected."[26]

Associated with neoclassicism,[25] the technique was first used in Igor Stravinsky's Octet (1923),[24] inspired by J. S. Bach and Giovanni Palestrina. However, according to Knud Jeppesen: "Bach's and Palestrina's points of departure are antipodal. Palestrina starts out from lines and arrives at chords; Bach's music grows out of an ideally harmonic background, against which the voices develop with a bold independence that is often breath-taking."[24]

According to Cunningham, linear harmony is "a frequent approach in the 20th century...[in which lines] are combined with almost careless abandon in the hopes that new 'chords' and 'progressions'...will result." It is possible with "any kind of line, diatonic or duodecuple".[26]

Dissonant counterpoint
[example needed]

Dissonant counterpoint was originally theorized by Charles Seeger as "at first purely a school-room discipline," consisting of species counterpoint but with all the traditional rules reversed. First species counterpoint must be all dissonances, establishing "dissonance, rather than consonance, as the rule," and consonances are "resolved" through a skip, not step. He wrote that "the effect of this discipline" was "one of purification". Other aspects of composition, such as rhythm, could be "dissonated" by applying the same principle.[27]

Seeger was not the first to employ dissonant counterpoint, but was the first to theorize and promote it. Other composers who have used dissonant counterpoint, if not in the exact manner prescribed by Charles Seeger, include Johanna Beyer, John Cage, Ruth Crawford-Seeger, Vivian Fine, Carl Ruggles, Henry Cowell, Carlos Chávez, John J. Becker, Henry Brant, Lou Harrison, Wallingford Riegger, and Frank Wigglesworth.[28]