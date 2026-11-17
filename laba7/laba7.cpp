/******************************
 * Автор:   Ли Даниил         *
 * Задание: Структуры и файлы *
 * Вариант: 3                 *
 *****************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

// Ограничение числа записей по условию лабораторной
constexpr int maxRecordCount = 100;
// Число полей строки: author;title;year;quantity;price
constexpr int bookFieldCount = 5;
// Нижняя граница года: начало европейского книгопечатания
constexpr int minPublicationYear = 1450;
// Разумная верхняя граница года издания
constexpr int maxPublicationYear = 2100;
// Количество экземпляров не может быть отрицательным
constexpr int minBookQuantity = 0;
// Цена не может быть отрицательной
constexpr double minBookPrice = 0.0;
// Признак "книга не найдена" для функций поиска
constexpr int noBookIndex = -1;
// Денежные значения выводим с двумя знаками после точки
constexpr int moneyPrecision = 2;

// Одна запись файла: все поля книги хранятся вместе
struct Book {
  // Фамилия автора, текст без символа ';'
  string authorSurname;
  // Название книги, может содержать пробелы
  string title;
  // Год издания
  int publicationYear;
  // Количество экземпляров
  int quantity;
  // Цена одного экземпляра в рублях
  double price;
};

// Убирает краевые пробелы и табуляции, чтобы " " считалось пустым полем
string trimSpaces(const string& text) {
  size_t firstIndex = text.find_first_not_of(" \t");

  if (firstIndex == string::npos) {
    return "";
  }
  size_t lastIndex = text.find_last_not_of(" \t");
  return text.substr(firstIndex, lastIndex - firstIndex + 1);
}

// Делит строку по ';', сохраняя пустое последнее поле:
// запись "A;B;1949;5;" должна быть отклонена, а не "починена"
vector<string> splitFields(const string& line) {
  vector<string> fieldList;
  size_t startIndex = 0;
  size_t separatorIndex;

  while (true) {
    separatorIndex = line.find(';', startIndex);

    if (separatorIndex == string::npos) {
      fieldList.push_back(line.substr(startIndex));
      break;
    }
    fieldList.push_back(line.substr(startIndex, separatorIndex - startIndex));
    startIndex = separatorIndex + 1;
  }
  return fieldList;
}

// Строгое чтение целого: "2.5" и "12abc" не должны стать числом
bool readInt(const string& text, int& value) {
  istringstream parser(text);
  int parsedValue = 0;
  char extraChar = ' ';

  if (!(parser >> parsedValue)) {
    return false;
  }
  // После числа допустимы только пробелы, но не посторонние символы
  if (parser >> extraChar) {
    return false;
  }
  value = parsedValue;
  return true;
}

// Строгое чтение конечного double: запятая вместо точки отклоняется
bool readDouble(const string& text, double& value) {
  istringstream parser(text);
  double parsedValue = 0.0;
  char extraChar = ' ';

  if (!(parser >> parsedValue) || !isfinite(parsedValue)) {
    return false;
  }
  if (parser >> extraChar) {
    return false;
  }
  value = parsedValue;
  return true;
}

// Разбирает и проверяет одну строку; book заполняется только при успехе
bool parseBook(const string& line, Book& book) {
  vector<string> fieldList = splitFields(line);

  if (static_cast<int>(fieldList.size()) != bookFieldCount) {
    return false;
  }
  string authorSurname = trimSpaces(fieldList[0]);
  string title = trimSpaces(fieldList[1]);

  if (authorSurname.empty() || title.empty()) {
    return false;
  }

  int publicationYear = 0;
  int quantity = 0;
  double price = 0.0;

  if (!readInt(fieldList[2], publicationYear)) {
    return false;
  }
  if (publicationYear < minPublicationYear || publicationYear > maxPublicationYear) {
    return false;
  }
  if (!readInt(fieldList[3], quantity)) {
    return false;
  }
  if (quantity < minBookQuantity) {
    return false;
  }
  if (!readDouble(fieldList[4], price)) {
    return false;
  }
  if (price < minBookPrice) {
    return false;
  }
  book.authorSurname = authorSurname;
  book.title = title;
  book.publicationYear = publicationYear;
  book.quantity = quantity;
  book.price = price;
  return true;
}

// Читает весь файл до вычислений; при любой ошибке печатает её и возвращает false
bool readBookList(const string& inputPath, vector<Book>& bookList) {
  ifstream input(inputPath);

  if (!input) {
    cout << "ERROR open\n";
    return false;
  }
  string line;
  int lineNumber = 0;

  // Условие цикла - успешное чтение строки, а не проверка eof
  while (getline(input, line)) {
    ++lineNumber;

    // Убираем возврат каретки, чтобы CRLF-файл читался как обычный
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    // Пустая строка между записями по условию считается ошибкой
    if (trimSpaces(line).empty()) {
      cout << "ERROR line " << lineNumber << "\n";
      return false;
    }
    if (static_cast<int>(bookList.size()) >= maxRecordCount) {
      cout << "ERROR line " << lineNumber << ": record limit exceeded\n";
      return false;
    }
    Book book{};
    if (!parseBook(line, book)) {
      cout << "ERROR line " << lineNumber << "\n";
      return false;
    }
    // В коллекцию запись попадает только после проверки всех полей
    bookList.push_back(book);
  }
  if (input.bad()) {
    cout << "ERROR read\n";
    return false;
  }
  return true;
}

// Индексы книг заданного автора; сравнение точное, с учётом регистра
vector<int> selectAuthorBookIndexList(const vector<Book>& bookList, const string& authorSurname) {
  vector<int> selectedIndexList;
  const int bookCount = static_cast<int>(bookList.size());

  for (int bookIndex = 0; bookIndex < bookCount; ++bookIndex) {
    if (bookList[bookIndex].authorSurname == authorSurname) {
      selectedIndexList.push_back(bookIndex);
    }
  }
  return selectedIndexList;
}

// Общая стоимость выборки: сумма quantity * price по каждой книге
double computeTotalCost(const vector<Book>& bookList,
  const vector<int>& selectedIndexList) {
  double totalCost = 0.0;
  const int selectionCount = static_cast<int>(selectedIndexList.size());

  for (int selectionIndex = 0; selectionIndex < selectionCount; ++selectionIndex) {
    const Book& book = bookList[selectedIndexList[selectionIndex]];

    totalCost += book.quantity * book.price;
  }
  return totalCost;
}

// Суммарное число экземпляров в выборке
int computeTotalCopies(const vector<Book>& bookList,
  const vector<int>& selectedIndexList) {
  int totalCopies = 0;
  const int selectionCount = static_cast<int>(selectedIndexList.size());

  for (int selectionIndex = 0; selectionIndex < selectionCount; ++selectionIndex) {
    const Book& book = bookList[selectedIndexList[selectionIndex]];
    totalCopies += book.quantity;
  }
  return totalCopies;
}

// Самая старая книга среди всех; при равных годах остаётся первая в файле
int findOldestBookIndex(const vector<Book>& bookList) {
  if (bookList.empty()) {
    return noBookIndex;
  }
  int oldestBookIndex = 0;
  const int bookCount = static_cast<int>(bookList.size());

  for (int bookIndex = 1; bookIndex < bookCount; ++bookIndex) {
    // Строгое "<" сохраняет первую запись при равенстве годов
    if (bookList[bookIndex].publicationYear < bookList[oldestBookIndex].publicationYear) {
      oldestBookIndex = bookIndex;
    }
  }
  return oldestBookIndex;
}

// Деньги форматируем единообразно для экрана и файла
string formatMoney(double amount) {
  ostringstream formatter;

  formatter << fixed << setprecision(moneyPrecision) << amount;
  return formatter.str();
}

// Одна запись в том же формате, что и во входном файле
string formatBookLine(const Book& book) {
  return book.authorSurname + ";" + book.title + ";" +
    to_string(book.publicationYear) + ";" + to_string(book.quantity) + ";" +
    formatMoney(book.price);
}

// Отчёт собирается один раз, поэтому экран и файл получат одинаковый текст
string buildReport(const vector<Book>& bookList, const string& authorSurname,
  const vector<int>& selectedIndexList, int totalCopies, double totalCost,
  int oldestBookIndex) {
  ostringstream report;
  report << "Author query: " << authorSurname << "\n";
  report << "Selected books (author;title;year;quantity;price):\n";
  const int selectionCount = static_cast<int>(selectedIndexList.size());
  if (selectionCount == 0) {
    report << "EMPTY\n";
  }
  else {
    for (int selectionIndex = 0; selectionIndex < selectionCount; ++selectionIndex) {
      const Book& book = bookList[selectedIndexList[selectionIndex]];
      report << selectionIndex + 1 << ". " << formatBookLine(book) << "\n";
    }
  }
  report << "Selected book count: " << selectionCount << "\n";
  report << "Total copies: " << totalCopies << "\n";
  report << "Total cost: " << formatMoney(totalCost) << "\n";
  report << "Oldest book: ";
  if (oldestBookIndex == noBookIndex) {
    report << "NONE\n";
  }
  else {
    report << formatBookLine(bookList[oldestBookIndex]) << "\n";
  }
  return report.str();
}

int main() {
  // --- Блок ввода ---
  string inputPath;
  string outputPath;
  string authorSurname;
  cout << "Input file path: ";
  if (!getline(cin, inputPath)) {
    cout << "ERROR input\n";
    return 1;
  }
  cout << "Output file path: ";
  if (!getline(cin, outputPath)) {
    cout << "ERROR input\n";
    return 1;
  }
  cout << "Author surname: ";
  if (!getline(cin, authorSurname)) {
    cout << "ERROR input\n";
    return 1;
  }
  authorSurname = trimSpaces(authorSurname);
  if (authorSurname.empty()) {
    cout << "ERROR empty author query\n";
    return 1;
  }
  vector<Book> bookList;
  if (!readBookList(inputPath, bookList)) {
    return 1;
  }

  // --- Блок расчёта ---
  vector<int> selectedIndexList = selectAuthorBookIndexList(bookList, authorSurname);
  int totalCopies = computeTotalCopies(bookList, selectedIndexList);
  double totalCost = computeTotalCost(bookList, selectedIndexList);
  int oldestBookIndex = findOldestBookIndex(bookList);

  // --- Блок вывода ---
  string reportText = buildReport(bookList, authorSurname, selectedIndexList,
    totalCopies, totalCost, oldestBookIndex);
  ofstream output(outputPath);
  if (!output) {
    cout << "ERROR output\n";
    return 1;
  }
  output << reportText;
  output.flush();
  // Проверяем успешность записи до вывода отчёта на экран
  if (!output) {
    cout << "ERROR write\n";
    return 1;
  }
  cout << reportText;
  return 0;
}