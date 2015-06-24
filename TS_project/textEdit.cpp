#include "mainwindow.h"

void MainWindow::on_textEdit_textChanged()
{
    if (tpl == ui->textEdit->toPlainText().length() || off_ct)
        return;
    off_ct = true;
    refresh_ur(false);
    if (tpl<ui->textEdit->toPlainText().length())
    {
        if (tpl+1==ui->textEdit->toPlainText().length())
        {
            QTextCursor cur (ui->textEdit->document());
            cur.setPosition(ui->textEdit->textCursor().position());
            cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);
            QString old_letter = cur.selectedText();
            cur.removeSelectedText();
            cur.insertText(old_letter,QTextCharFormat ());
            if (ui->m_stress->isChecked() && is_job_done)
            {
                QString wrong_char = "?!., ;:\'\"\\/+><@#~`$%^&*(){}|\n\r\t\a\?";
                cur.clearSelection();
                cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);
                if (wrong_char.contains(cur.selectedText().at(0)) || cur.selectedText().at(0).unicode()==8233)
                {
                    cur.setPosition(ui->textEdit->textCursor().position()-1);
                    cur.clearSelection();
                    while (cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor) &&
                           !wrong_char.contains(cur.selectedText().at(0)) && cur.selectedText().at(0).unicode()!=8233);
                    QString test_word = cur.selectedText().toLower();
                    QChar end_of_buf;
                    if (wrong_char.contains(cur.selectedText()[0]) || cur.selectedText()[0].unicode()==8233)
                    {
                        end_of_buf = cur.selectedText().at(0);
                        test_word.remove(0,1);
                    }
                    int finded =0;
                    if ((finded=find_in_data(test_word,1))==1)
                    {
                        cur.removeSelectedText();
                        cur.insertText(end_of_buf);
                        cur.insertHtml("<b><u><font color='red'>" + test_word + "</font></b></u>");
                    }
                    else if (finded==2)
                    {
                        cur.removeSelectedText();
                        cur.insertText(end_of_buf);
                        cur.insertHtml("<b><font color='blue'>" + test_word + "</font></b>");
                    }
                    if (show_results)
                        table_show_results();
                }
            }
            oper_stack.insert(oper_stack.begin(),1,
                              oper_info(1,ui->textEdit->textCursor().position(),0,
                                        old_letter    ));
        }
        else
        {
            if (ui->m_stress->isChecked() && is_job_done)
            {
                QTextCursor cur (ui->textEdit->document());
                cur.setPosition(ui->textEdit->textCursor().position());
                cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::MoveAnchor,ui->textEdit->toPlainText().length()-tpl);
                QString word;
                QString wrong_char = "?!., ;:\'\"\\/+><@#~`$%^&*(){}|\n\r\t\a\?";
                int founded;
                for (int i=ui->textEdit->toPlainText().length()-tpl; i>0; i--)
                {
                    cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
                    if (wrong_char.contains(cur.selectedText()[cur.selectedText().length()-1]))
                    {
                        word = cur.selectedText();
                        if ((founded=find_in_data(word.left(word.length()-1),1))==1)
                        {
                            cur.removeSelectedText();
                            cur.insertHtml(QString("<b><u><font color='red'>" + word.left(word.length()-1) + "</font></b></u>%1").arg(word[word.length()-1]));
                        }
                        else if (founded==2)
                        {
                            cur.removeSelectedText();
                            cur.insertHtml(QString("<b><font color='blue'>" + word.left(word.length()-1) + "</font></b>%1").arg(word[word.length()-1]));
                        }
                        cur.clearSelection();
                    }
                }
                if (show_results)
                    table_show_results();
            }
            oper_stack.insert(oper_stack.begin(),1,
                              oper_info(1,ui->textEdit->textCursor().position(),1,
                                        fake_text,ui->textEdit->toHtml()    ));
        }
    }
    else
    {
        if (tpl-1==ui->textEdit->toPlainText().length())
        {
            QString wrong_char = "?!., ;:\'\"\\/+><@#~`$%^&*(){}|\n\r\t\a\?";
            if ((ui->m_stress->isChecked() && is_job_done) &&
                    ((hand_cur==ui->textEdit->textCursor().position() && fake_chars.second.unicode()!=8233 && !wrong_char.contains(fake_chars.second)) ||
                     (fake_chars.first.unicode()!=8233 && !wrong_char.contains(fake_chars.first))))
            {
                QTextCursor cur (ui->textEdit->document());
                cur.setPosition(ui->textEdit->textCursor().position());
                cur.clearSelection();
                while (cur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor) &&
                       !wrong_char.contains(cur.selectedText().at(0)) && cur.selectedText().at(0).unicode()!=8233);
                if (wrong_char.contains(cur.selectedText()[0]) || cur.selectedText()[0].unicode()==8233)
                    cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
                cur.clearSelection();
                if (cur.position()==0)
                    cur.movePosition(QTextCursor::NextCharacter);
                int finded =cur.position();

                while (cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor) &&
                       !wrong_char.contains(cur.selectedText().at(cur.selectedText().length()-1)) &&
                       cur.selectedText().at(cur.selectedText().length()-1).unicode()!=8233);
                if (cur.selectedText().length()!=0)
                {
                    QString test_word = cur.selectedText().toLower();
                    if (wrong_char.contains(cur.selectedText()[cur.selectedText().length()-1]) ||
                            cur.selectedText()[cur.selectedText().length()-1].unicode()==8233 ||
                            cur.selectedText()[cur.selectedText().length()-1].unicode()==0)
                    {
                        test_word.remove(cur.selectedText().length()-1,1);
                        cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);
                    }
                    if (wrong_char.contains(cur.selectedText()[0]) || cur.selectedText()[0].unicode()==8233 || cur.selectedText()[0].unicode()==0)
                        test_word.remove(0,1);
                    int colors = 1;
                    QTextCursor cur2 = cur;
                    cur2.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);
                    if (cur2.charFormat().foreground().color()==QColor("red") || cur2.charFormat().foreground().color()==QColor("blue"))
                    {
                        if (hand_cur!=ui->textEdit->textCursor().position())
                            test_word.insert(hand_cur-finded-1,fake_chars.first);
                        else
                            test_word.insert(hand_cur-finded,fake_chars.second);
                        colors = -1;
                    }
                    if ((finded=find_in_data(test_word,colors))==1)
                    {
                        cur.removeSelectedText();
                        if (colors==1)
                            cur.insertHtml("<b><u><font color='red'>" + test_word + "</font></b></u>");
                        else
                            cur.insertText(test_word,QTextCharFormat ());
                    }
                    else if (finded==2)
                    {
                        cur.removeSelectedText();
                        if (colors==1)
                            cur.insertHtml("<b><font color='blue'>" + test_word + "</font></b>");
                        else
                            cur.insertText(test_word,QTextCharFormat ());
                    }
                    if (show_results && finded!=-1)
                        table_show_results();
                }
            }
            if (hand_cur!=ui->textEdit->textCursor().position())
                oper_stack.insert(oper_stack.begin(),1,
                              oper_info(1,ui->textEdit->textCursor().position(),2,fake_chars.first));
            else
                oper_stack.insert(oper_stack.begin(),1,
                              oper_info(1,ui->textEdit->textCursor().position(),2,fake_chars.second));

            if (ui->textEdit->textCursor().position()<ui->textEdit->toPlainText().length() && ui->textEdit->textCursor().position()>0)
            {
                fake_chars.first = ui->textEdit->toPlainText().at(ui->textEdit->textCursor().position()-1);
                fake_chars.second = ui->textEdit->toPlainText().at(ui->textEdit->textCursor().position());
            }
        }
        else
        {
            if (ui->m_stress->isChecked() && is_job_done)
            {
                QString word;
                QString wrong_char = "?!., ;:\'\"\\/+><@#~`$%^&*(){}|\n\r\t\a\?";
                for (int i=0; i<hand_text.length(); i++)
                {
                    word += hand_text[i];
                    if (wrong_char.contains(word[word.length()-1]))
                    {
                        find_in_data(word.left(word.length()-1),-1);
                        word.clear();
                    }
                }
                if (!word.isEmpty())
                    find_in_data(word,-1);
                if (show_results)
                    table_show_results();
            }
            oper_stack.insert(oper_stack.begin(),1,
                              oper_info(1,ui->textEdit->textCursor().position(),1,
                                        fake_text,ui->textEdit->toHtml()    ));
        }
    }
    off_ct = false;
    hand_cur=ui->textEdit->textCursor().position();
    fake_text = ui->textEdit->toHtml();
    tpl = ui->textEdit->toPlainText().length();
    barl->setText(QString("%1 символов, %2 без пробелов").arg(tpl).arg(tpl-ui->textEdit->toPlainText().count(' ')));
}
