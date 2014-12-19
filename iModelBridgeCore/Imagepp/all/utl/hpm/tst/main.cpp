// Ce fichier de test est séparé en quatre grandes sections:
// 1 - Déclaration et définition d'une classe pour faire des objets
//     quelconques à manipuler
// 2 - Déclaration et définition d'une classe pour stocker des objets
// 3 - Définition des fonctions qui testent chacune un aspect particulier
// 4 - La fonction main() qui contrôle le déroulement du programme de test.


// Inclusions

#include <iostream>

#include <ImagePP/all/h/string.h>

#include <ImagePP/all/h/HPMPersistentObject.h>
#include <ImagePP/all/h/HPMObjectStore.h>
#include <ImagePP/all/h/HPMVirtualPtr.h>
#include <ImagePP/all/h/HPMObjectLog.h>

// *************************************************************************
// Première section:  Classe d'objets à manipuler "DummyClass"
// *************************************************************************

class DummyClass : public HPMPersistentObject
    {
    HPM_DECLARE_CLASS(999);

public:

    DummyClass(const char* text);
    DummyClass();
    virtual                    ~DummyClass();

    void                    Dummy();
    void                    Print();
    const char*             GetText() const {
        return dummyString;
        }

protected:

private:

    char dummyString[80];

    };

// Méthodes de la classe DummyClass

HPM_REGISTER_CLASS(DummyClass, HPMPersistentObject);

DummyClass::DummyClass()
    : HPMPersistentObject()
    {
    cout << "  (construction par défaut)" << endl;
    }

DummyClass::DummyClass(const char* text)
    : HPMPersistentObject()
    {
    strcpy(dummyString, text);
    cout << "  (construction de l'objet: " << dummyString << ")" << endl;
    }

DummyClass::~DummyClass()
    {
    cout << "  (destruction de l'objet: " << dummyString << ")" << endl;
    }

void DummyClass::Dummy()
    {
    // Dummy! :-)
    }

void DummyClass::Print()
    {
    cout << "(" << hex << (long)this << ") " << dummyString << endl;
    }

// *************************************************************************
// Deuxième section: classe de stockage
// *************************************************************************

#include <ImagePP/all/h/HPMStoreFactory.h>

class DummyStore : public HPMObjectStore
    {
public:
    DummyStore(HPMObjectLog* pi_pLog);
    DummyStore(void*, HPMObjectLog*);
    virtual ~DummyStore();
    virtual HPMObjectStore& operator<<(HPMPersistentObject& pi_rObj);
    virtual HPMObjectStore& operator>>(HPMPersistentObject& po_rObj);
    virtual bool Remove(HPMPersistentObject* pi_pObj);
    virtual bool IsReadOnly() const {
        return false;
        }
    virtual HPMPersistentObject* Load(HPMObjectID pi_ObjectID);
    virtual HPMGenericVirtualPtr LoadVirtual(HPMObjectID pi_ObjectID);
    virtual bool Lock(HPMObjectID, bool) {
        return false;
        }
    virtual void Unlock(HPMObjectID) {  }
    HPMObjectStore& operator<<(HPMPersistentObject* pi_pObj);
    HPMObjectStore& operator<<(HPMGenericVirtualPtr pi_pObj);
    HPMObjectStore& operator>>(HPMPersistentObject*& po_rpObj);
    HPMObjectStore& operator>>(HPMGenericVirtualPtr& pi_rpObj);
    virtual HPMGenericClassIterator* CreateClassIterator(long) {
        return 0;
        }
private:
    DummyStore(const DummyStore&);
    DummyStore& operator=(const DummyStore&);
    uint32_t m_objectCounter;
    };

HPM_REGISTER_STORE_CLASS(DummyStore, 1)

DummyStore::DummyStore(HPMObjectLog* pi_pLog)
    : HPMObjectStore(pi_pLog)
    {
    m_objectCounter = 0;
    }

DummyStore::DummyStore(void*, HPMObjectLog* pi_pLog)
    : HPMObjectStore(pi_pLog)
    {
    m_objectCounter = 0;
    }

DummyStore::~DummyStore()
    {
    }

HPMPersistentObject* DummyStore::Load(HPMObjectID pi_ObjectID)
    {
    FILE* pFile;
    char text[80];
    sprintf(text, "d:\\temp\\%d.$$$", pi_ObjectID);
    pFile = fopen(text, "rb+");
    HASSERT(pFile);
    fread(text, 80, 1, pFile);
    fclose(pFile);
    DummyClass* pObj = new DummyClass(text);
    return pObj;
    }

HPMGenericVirtualPtr DummyStore::LoadVirtual(HPMObjectID pi_ObjectID)
    {
    return HPMGenericVirtualPtr(0, false, this, 0, pi_ObjectID);
    }

HPMObjectStore& DummyStore::operator<<(HPMPersistentObject& pi_rObj)
    {
    FILE* pFile;
    char text[80];
    pi_rObj.SetID(m_objectCounter);
    sprintf(text, "d:\\temp\\%d.$$$", m_objectCounter++);
    pFile = fopen(text, "wb+");
    HASSERT(pFile);
    const char* pText = ((DummyClass*)&pi_rObj)->GetText();
    fwrite(pText, 80, 1, pFile);
    fclose(pFile);
    return *this;
    }

HPMObjectStore& DummyStore::operator<<(HPMPersistentObject* pi_pObj)
    {
    *this << *pi_pObj;
    return *this;
    }

HPMObjectStore& DummyStore::operator<<(HPMGenericVirtualPtr pi_pObj)
    {
    *this << pi_pObj.GetAddress();
    return *this;
    }

HPMObjectStore& DummyStore::operator>>(HPMPersistentObject& po_rObj)
    {
    // Not used
    return *this;
    }

HPMObjectStore& DummyStore::operator>>(HPMPersistentObject*& po_rpObj)
    {
    // Not used
    return *this;
    }

HPMObjectStore& DummyStore::operator>>(HPMGenericVirtualPtr& pi_rpObj)
    {
    // Not used
    return *this;
    }

bool DummyStore::Remove(HPMPersistentObject* pi_pObj)
    {
    // Not used
    return false;
    }

// *************************************************************************
// Troisième section: fonctions de testage par thème
// *************************************************************************

// Test du fonctionnement de la liste MRU

void TestMRUList()
    {
    // Fonctionnement: on se crée un manager qui peut prendre 20 objets.
    // On s'en cree 10, avec un pointeur pour chacun, et on s'en sert de
    // différentes façons, tout en imprimant le contenu de la liste MRU.

    cout << "***************************" << endl;
    cout << "TEST LISTE MRU" << endl;
    cout << "***************************" << endl;

    // Restorer et manager
    cout << "Creation du store et du log..." << endl;
    HPMObjectLog MyLog(20);
    DummyStore MyStore(&MyLog);

        {
        // Les objets et les pointeurs
        // Deux pointeurs sont créés dans un scope plus restreint pour pouvoir
        // les détruire avant les autres
        cout << "Creations des objets et pointeurs..." << endl;
        HPMVirtualPtr<DummyClass> ptr1(new DummyClass("un"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr2(new DummyClass("deux"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr3(new DummyClass("trois"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr4(new DummyClass("quatre"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr5(new DummyClass("cinq"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr6(new DummyClass("six"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr7(new DummyClass("sept"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr8(new DummyClass("huit"), &MyStore);
            {
            HPMVirtualPtr<DummyClass> ptr9(new DummyClass("neuf"), &MyStore);
            HPMVirtualPtr<DummyClass> ptr10(new DummyClass("dix"), &MyStore);


            MyLog.PrintState();  // De quoi a l'air la MRU maintenant?
            cout << "Appuyez sur une touche..." << endl;
            getchar();

            // Je me sers du pointeur #7 pis du #1 pis du #3, ils doivent
            // se trouver en tete de la liste en ordre 3 - 1 - 7 ...
            cout << "J'utilise les pointeurs sur les objets suivants:" << endl;
            ptr7->Print();
            ptr1->Print();
            ptr3->Print();
            MyLog.PrintState();
            cout << "Appuyez sur une touche..." << endl;
            getchar();

            // Renversement de l'ordre...
            cout << "J'utilise les pointeurs du dixieme au premier" << endl;
            ptr10->Print();
            ptr9->Print();
            ptr8->Print();
            ptr7->Print();
            ptr6->Print();
            ptr5->Print();
            ptr4->Print();
            ptr3->Print();
            ptr2->Print();
            ptr1->Print();
            MyLog.PrintState();
            cout << "Appuyez sur une touche..." << endl;
            getchar();

            // On essaie maintenant le cinquieme et le neuvieme d'une facon differente
            cout << "Pointeurs #5 et #9 utilises autrement..." << endl;
            DummyClass* ptrTest = (DummyClass*)ptr5;
            (*ptr9).Dummy();
            MyLog.PrintState();
            cout << "Appuyez sur une touche..." << endl;
            getchar();

            cout << "Destruction des pointeurs #9 et #10" << endl;
            }
        MyLog.PrintState();
        cout << "Appuyez sur une touche..." << endl;
        getchar();

        // Effacement des objets par destruction des pointeurs avant de
        // terminer cette fonction
        cout << "Destruction de tous les pointeurs" << endl;
        }
    MyLog.PrintState();
    cout << "Appuyez sur une touche..." << endl;
    getchar();
    }

// Test du discard et restore des objets...

void TestDiscardRestore()
    {
    // Fonctionnement: on se crée un manager qui peut prendre 5 objets.
    // On s'en cree 10, avec un pointeur pour chacun, et on s'en sert de
    // différentes façons, tout en imprimant le contenu de la liste MRU
    // et des objets...

    cout << "****************************" << endl;
    cout << "TEST DU DISCARD et RESTORE" << endl;
    cout << "****************************" << endl;

    // Restorer et manager
    cout << "Creation du store et du log (max 5)..." << endl;
    HPMObjectLog MyLog(5);
    DummyStore MyStore(&MyLog);

        {
        // Les objets et les pointeurs
        // Certains pointeurs sont définis dans un scope plus restreint
        cout << "Creations des objets et pointeurs... (10 de chaque)" << endl;
        HPMVirtualPtr<DummyClass> ptr1(new DummyClass("un"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr2(new DummyClass("deux"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr3(new DummyClass("trois"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr4(new DummyClass("quatre"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr5(new DummyClass("cinq"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr6(new DummyClass("six"), &MyStore);
        HPMVirtualPtr<DummyClass> ptr7(new DummyClass("sept"), &MyStore);
            {
            HPMVirtualPtr<DummyClass> ptr8(new DummyClass("huit"), &MyStore);
            HPMVirtualPtr<DummyClass> ptr9(new DummyClass("neuf"), &MyStore);
            HPMVirtualPtr<DummyClass> ptr10(new DummyClass("dix"), &MyStore);

            // De quoi a l'air la liste MRU maintenant?
            MyLog.PrintState();
            cout << "Appuyez sur une touche..." << endl;
            getchar();

            // Test du locking

            cout << "ptr7 est locke" << endl;
            ptr7.Lock();
            MyLog.PrintState();
            cout << "Appuyez sur une touche..." << endl;
            getchar();

            // Je me sers du pointeur #6 pis du #1 pis du #3, ils doivent
            // se trouver en tete de la liste en ordre 3 - 1 - 6 ...
            cout << "J'utilise les pointeurs sur les objets 6 - 1 - 3:" << endl;
            ptr6->Print();
            getchar();
            ptr1->Print();
            getchar();
            ptr3->Print();
            getchar();
            MyLog.PrintState();
            cout << "Appuyez sur une touche..." << endl;
            getchar();

            // Renversement de l'ordre...
            cout << "J'utilise les pointeurs du dixieme au premier" << endl;
            ptr10->Print();
            ptr9->Print();
            ptr8->Print();
            ptr7->Print();
            ptr6->Print();
            ptr5->Print();
            ptr4->Print();
            ptr3->Print();
            ptr2->Print();
            ptr1->Print();
            MyLog.PrintState();
            cout << "Appuyez sur une touche..." << endl;
            getchar();
            cout << "Etat des pointeurs à ce moment:" << endl;
            ptr10.PrintState();
            ptr9.PrintState();
            ptr8.PrintState();
            ptr7.PrintState();
            ptr6.PrintState();
            ptr5.PrintState();
            ptr4.PrintState();
            ptr3.PrintState();
            ptr2.PrintState();
            ptr1.PrintState();
            cout << "Appuyez sur une touche..." << endl;
            getchar();

            // Effacement des pointeurs 1, 5 et 9...
            cout << "Pointeurs #8, #9 et #10 detruits" << endl;
            }
        MyLog.PrintState();
        cout << "Appuyez sur une touche..." << endl;
        getchar();

        // Effacement des autres objets avant de terminer cette fonction
        cout << "ptr7 unlocke et destruction des autres pointeurs" << endl;
        ptr7.Unlock();
        }
    MyLog.PrintState();
    cout << "Appuyez sur une touche..." << endl;
    getchar();
    }


void TestDuplicates()
    {
    // Ici on se cree un seul objet mais 10 pointeurs sur celui-ci.

    cout << "************************************" << endl;
    cout << "TEST DE LA DUPLICATION DES POINTEURS" << endl;
    cout << "************************************" << endl;

    // Restorer et manager
    cout << "Creation du store et du log..." << endl;
    HPMObjectLog MyLog(5);
    DummyStore MyStore(&MyLog);

    // Un objet et les pointeurs
    cout << "Creations d'un objet et des pointeurs..." << endl;
    HPMVirtualPtr<DummyClass> ptr1(new DummyClass("Le Seul et l'Unique"), &MyStore);
        {
        HPMVirtualPtr<DummyClass> ptr2(ptr1);
        HPMVirtualPtr<DummyClass> ptr3(ptr1);
        HPMVirtualPtr<DummyClass> ptr4(ptr2);
        HPMVirtualPtr<DummyClass> ptr5(ptr2);
        HPMVirtualPtr<DummyClass> ptr6(ptr3);
        HPMVirtualPtr<DummyClass> ptr7(ptr3);
        HPMVirtualPtr<DummyClass> ptr8(ptr4);
        HPMVirtualPtr<DummyClass> ptr9(ptr4);
        HPMVirtualPtr<DummyClass> ptr10(ptr5);

        // La liste MRU ne devrait contenir qu'une seule entree
        MyLog.PrintState();
        cout << "Appuyez sur une touche..." << endl;
        getchar();

        // Tous les pointeurs doivent donner le meme objet:
        cout << "Contenu de l'objet via pointeurs #1 a #10" << endl;
        ptr1->Print();
        ptr2->Print();
        ptr3->Print();
        ptr4->Print();
        ptr5->Print();
        ptr6->Print();
        ptr7->Print();
        ptr8->Print();
        ptr9->Print();
        ptr10->Print();
        MyLog.PrintState();
        cout << "Appuyez sur une touche..." << endl;
        getchar();

        // Creation d'un second objet, il apparait en tete de la MRU
        cout << "Creation d'un nouvel objet et du ptr #11" << endl;
        HPMVirtualPtr<DummyClass> ptr11(new DummyClass("Le Second"), &MyStore);
        MyLog.PrintState();
        cout << "Appuyez sur une touche..." << endl;
        getchar();
        cout << "Utilisation du pointeur #10" << endl;
        ptr10->Dummy();
        MyLog.PrintState();
        cout << "Appuyez sur une touche..." << endl;
        getchar();

        // On utilise l'operateur d'assignation pour creer une copie de pointeur
        cout << "Ptr8 = ptr11, contenu de l'objet pointe par #8:" << endl;
        ptr8 = ptr11;
        ptr8->Print();
        MyLog.PrintState();
        cout << "Appuyez sur une touche..." << endl;
        getchar();

        // Destruction des objets
        cout << "Ptr8 = ptr11 = 0, objet doit être détruit" << endl;
        ptr8 = 0;
        ptr11 = 0;
        MyLog.PrintState();
        cout << "Appuyez sur une touche..." << endl;
        getchar();

        cout << "Destruction des pointeurs sauf #1..." << endl;
        }
    MyLog.PrintState();
    cout << "Appuyez sur une touche..." << endl;
    getchar();

    cout << "Ptr1 = 0, objet doit être détruit" << endl;
    ptr1 = 0;
    MyLog.PrintState();
    cout << "Appuyez sur une touche..." << endl;
    getchar();
    }

void TestSmartMgmt()
    {
    // Ici on teste la partie "smart pointer".  Les traces dans les constructeurs et
    // destructeurs seront utilisées.

    cout << "*************************************" << endl;
    cout << "TEST DE LA GESTION DES OBJETS POINTES" << endl;
    cout << "*************************************" << endl;

    cout << "Creation d'un objet, ensuite creation d'un pointeur sur celui-ci" << endl;
    DummyClass* pObj = new DummyClass("123");
        {
        HPMVirtualPtr<DummyClass> ptr1(pObj);
            {
            cout << "Creation de ptr2 en copie du premier" << endl;
            HPMVirtualPtr<DummyClass> ptr2(ptr1);
                {
                cout << "Creation de ptr3, mais avec adresse de l'objet" << endl;
                HPMVirtualPtr<DummyClass> ptr3(pObj);
                cout << "Etat des pointeurs a ce moment:" << endl;
                ptr1.PrintState();
                ptr2.PrintState();
                ptr3.PrintState();
                cout << "Appuyez sur une touche..." << endl;
                getchar();
                cout << "Destruction de ptr3" << endl;
                }
            cout << "Appuyez sur une touche..." << endl;
            getchar();
            cout << "Destruction de ptr2" << endl;
            }
        cout << "Appuyez sur une touche..." << endl;
        getchar();
        cout << "Destruction de ptr1" << endl;
        }
    cout << "Appuyez sur une touche..." << endl;
    getchar();

    cout << "Creation d'un objet, ensuite creation d'un pointeur temporaire protecteur sur celui-ci" << endl;
    DummyClass* pObj2 = new DummyClass("Protege");
        {
        HPMVirtualPtr<DummyClass> tempPtr(pObj2, true);
        tempPtr->Dummy();
        cout << "Pointeur temporaire detruit" << endl;
        }
    cout << "Appuyez sur une touche..." << endl;
    getchar();
    cout << "Creation d'un autre pointeur (ptr4) sur cet objet" << endl;
    HPMVirtualPtr<DummyClass> ptr4(pObj2);
    cout << "Creation d'un autre pointeur (ptr5) pointant sur un nouvel objet" << endl;
    HPMVirtualPtr<DummyClass> ptr5(new DummyClass("nouvel objet"));
    cout << "ptr4 = ptr5, objet 'Protege' doit etre detruit" << endl;
    ptr4 = ptr5;
    cout << "Appuyez sur une touche..." << endl;
    getchar();
    cout << "ptr5 = 0" << endl;
    ptr5 = 0;
    cout << "ptr4 = 0, nouvel objet doit etre detruit" << endl;
    ptr4 = 0;
    cout << "Appuyez sur une touche..." << endl;
    getchar();
    }

// *************************************************************************
// Quatrième section: fonction "main"
// *************************************************************************

int main(int argc, char** argv)
    {
    TestMRUList();

    TestDiscardRestore();

    TestDuplicates();

    TestSmartMgmt();

    return 0;
    }
