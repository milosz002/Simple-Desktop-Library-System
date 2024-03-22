This repository contains a simple desktop library system project, written in C and utilizing the GTK library for creating a graphical user interface. The program allows for loading a library database from text files, where each line includes a book title, author, quantity in stock, and price, with each piece of data separated by semicolons. Once the library database is loaded, it can be displayed directly within the application. Users have the ability to add new entries, edit existing ones, and remove selected items from the library. Additionally, the application supports saving the edited library database back to a text file, facilitating easy updates and management of library inventories.

Below is an example of a text file containing sample data to be loaded into the program:
```
The Hagwood Books;Robin Jarvis;15;25,99;
The Halfblood Chronicles;Mercedes Lackey and Andre Norton;10;12,99;
Harry Potter and the Philosopher's Stone;J. K. Rowling;12;39,99;
Harry Potter and the Chamber of Secrets;J. K. Rowling;12;39,99;
His Dark Materials series;Philip Pullman;2;99,99;
The Hobbit;J. R. R. Tolkien;15;44,99;
The Hounds of the Morrigan;Pat O'Shea;11;64,99;
The House on the Borderland;William Hope Hodgson;15;25,99;
House of Night;P.C. Cast;10;12,99;
The House with a Clock in Its Walls;John Bellairs;3;5,99;
Howl's Moving Castle;Diana Wynne Jones;12;89,99;
The Hundred Thousand Kingdoms;N. K. Jemisin;3;3,99;
The Hunger Games;Suzanne Collins;10;34,99;
Hush, Hush;Becca Fitzpatrick;11;24,99;
```
